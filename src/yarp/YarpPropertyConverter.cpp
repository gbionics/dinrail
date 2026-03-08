// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "YarpPropertyConverter.h"

#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>

#include <cstdint>
#include <limits>

namespace dinrail
{

namespace
{

bool isGroupEntry(const yarp::os::Bottle& entry)
{
    if (entry.size() < 2)
    {
        return false;
    }

    for (int i = 1; i < entry.size(); ++i)
    {
        const yarp::os::Value& value = entry.get(i);
        if (!value.isList())
        {
            return false;
        }

        yarp::os::Bottle* child = value.asList();
        if (child == nullptr || child->size() < 2 || !child->get(0).isString())
        {
            return false;
        }
    }

    return true;
}

void convertEntryValue(const std::string& key,
                       const yarp::os::Value& value,
                       Parameters& output)
{
    if (value.isBool())
    {
        output.put(key, value.asBool());
        return;
    }

    if (value.isInt64())
    {
        const auto yarpValue = value.asInt64();
        if (yarpValue < static_cast<std::int64_t>(std::numeric_limits<int>::min())
            || yarpValue > static_cast<std::int64_t>(std::numeric_limits<int>::max()))
        {
            return;
        }

        output.put(key, static_cast<int>(yarpValue));
        return;
    }

    if (value.isInt32())
    {
        output.put(key, value.asInt32());
        return;
    }

    if (value.isFloat64())
    {
        output.put(key, value.asFloat64());
        return;
    }

    if (value.isString())
    {
        output.put(key, value.asString());
        return;
    }

    if (!value.isList())
    {
        return;
    }

    yarp::os::Bottle* list = value.asList();
    if (list == nullptr)
    {
        return;
    }

    if (list->size() == 0)
    {
        output.put(key, std::vector<std::string>{});
        return;
    }

    bool allInt = true;
    bool allFloat = true;
    bool allString = true;
    bool allBool = true;
    for (int i = 0; i < list->size(); ++i)
    {
        const yarp::os::Value& element = list->get(i);
        const bool isInt = element.isInt32() || element.isInt64();
        allInt = allInt && isInt;
        allFloat = allFloat && element.isFloat64();
        allString = allString && element.isString();
        allBool = allBool && element.isBool();
    }

    if (allBool)
    {
        std::vector<bool> converted;
        converted.reserve(static_cast<std::size_t>(list->size()));
        for (int i = 0; i < list->size(); ++i)
        {
            converted.push_back(list->get(i).asBool());
        }
        output.put(key, converted);
        return;
    }

    if (allInt)
    {
        std::vector<int> converted;
        converted.reserve(static_cast<std::size_t>(list->size()));
        for (int i = 0; i < list->size(); ++i)
        {
            if (list->get(i).isInt64())
            {
                const auto yarpValue = list->get(i).asInt64();
                if (yarpValue < static_cast<std::int64_t>(std::numeric_limits<int>::min())
                    || yarpValue > static_cast<std::int64_t>(std::numeric_limits<int>::max()))
                {
                    return;
                }

                converted.push_back(static_cast<int>(yarpValue));
            }
            else
            {
                converted.push_back(list->get(i).asInt32());
            }
        }
        output.put(key, converted);
        return;
    }

    if (allFloat)
    {
        std::vector<double> converted;
        converted.reserve(static_cast<std::size_t>(list->size()));
        for (int i = 0; i < list->size(); ++i)
        {
            converted.push_back(list->get(i).asFloat64());
        }
        output.put(key, converted);
        return;
    }

    if (allString)
    {
        std::vector<std::string> converted;
        converted.reserve(static_cast<std::size_t>(list->size()));
        for (int i = 0; i < list->size(); ++i)
        {
            converted.push_back(list->get(i).asString());
        }
        output.put(key, converted);
        return;
    }

    // Fallback for mixed/unsupported list element types.
    output.put(key, list->toString());
}

void convertEntryToParameters(const yarp::os::Bottle& entry, Parameters& output)
{
    if (entry.size() < 2 || !entry.get(0).isString())
    {
        return;
    }

    const std::string key = entry.get(0).asString();
    if (!isGroupEntry(entry))
    {
        convertEntryValue(key, entry.get(1), output);
        return;
    }

    Parameters& group = output.addGroup(key);
    for (int i = 1; i < entry.size(); ++i)
    {
        yarp::os::Bottle* child = entry.get(i).asList();
        if (child == nullptr)
        {
            continue;
        }

        convertEntryToParameters(*child, group);
    }
}

void convertBottleToParameters(const yarp::os::Bottle& bottle, Parameters& output)
{
    for (int i = 0; i < bottle.size(); ++i)
    {
        const yarp::os::Value& value = bottle.get(i);
        if (!value.isList())
        {
            continue;
        }

        yarp::os::Bottle* entry = value.asList();
        if (entry == nullptr)
        {
            continue;
        }

        convertEntryToParameters(*entry, output);
    }
}

} // namespace

yarp::os::Property YarpPropertyConverter::toYarpProperty(const Parameters& dinrailParams)
{
    yarp::os::Property yarpProp;
    convertRecursive(dinrailParams, yarpProp);
    return yarpProp;
}

Parameters YarpPropertyConverter::toDinrailParameters(const yarp::os::Property& yarpProp)
{
    Parameters output;

    yarp::os::Bottle bottle;
    bottle.fromString(yarpProp.toString());
    convertBottleToParameters(bottle, output);

    return output;
}

void YarpPropertyConverter::convertRecursive(const Parameters& dinrailParams, yarp::os::Property& yarpProp)
{
    // First, copy all data (non-group) keys
    std::vector<std::string> dataKeys = dinrailParams.getValueKeys();
    for (const auto& key : dataKeys)
    {
        if (dinrailParams.check<bool>(key))
        {
            yarpProp.put(key, dinrailParams.find(key).as<bool>() ? 1 : 0);
            continue;
        }

        if (dinrailParams.check<int>(key))
        {
            yarpProp.put(key, dinrailParams.find(key).as<int>());
            continue;
        }

        if (dinrailParams.check<double>(key))
        {
            yarpProp.put(key, dinrailParams.find(key).as<double>());
            continue;
        }

        if (dinrailParams.check<std::string>(key))
        {
            yarpProp.put(key, dinrailParams.find(key).as<std::string>());
        }
    }
    
    // Then, recursively convert all nested groups
    std::vector<std::string> groupKeys = dinrailParams.getGroupKeys();
    for (const auto& key : groupKeys)
    {
        const auto& nestedGroup = dinrailParams.findGroup(key);
        if (!nestedGroup.isNull())
        {
            yarp::os::Property& yarpNestedGroup = yarpProp.addGroup(key);
            convertRecursive(nestedGroup, yarpNestedGroup);
        }
    }
}

} // namespace dinrail
