// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpPropertyConverter.h>

#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/os/Value.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <stdexcept>

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

std::string durationToBipedalYarpString(const std::chrono::nanoseconds& duration)
{
    using namespace std::chrono_literals;

    constexpr std::size_t zeroPadding = 2;
    constexpr std::size_t zeroPaddingMicroseconds = 6;

    std::string hours
        = std::to_string(std::chrono::duration_cast<std::chrono::hours>(duration).count());
    std::string minutes
        = std::to_string(std::chrono::duration_cast<std::chrono::minutes>(duration % 1h).count());
    std::string seconds
        = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(duration % 1min).count());
    std::string microseconds = std::to_string(
        std::chrono::duration_cast<std::chrono::microseconds>(duration % 1s).count());

    std::string timeString
        = std::string(zeroPadding - std::min(zeroPadding, hours.length()), '0') + hours + ":";
    timeString += std::string(zeroPadding - std::min(zeroPadding, minutes.length()), '0')
                  + minutes + ":";
    timeString += std::string(zeroPadding - std::min(zeroPadding, seconds.length()), '0')
                  + seconds + ":";
    timeString += std::string(zeroPaddingMicroseconds
                                  - std::min(zeroPaddingMicroseconds, microseconds.length()),
                              '0')
                  + microseconds;

    return timeString;
}

void convertEntryValue(const std::string& key, const yarp::os::Value& value, Parameters& output)
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
            throw std::runtime_error("Cannot convert int64 value for key '" + key
                                     + "' to dinrail int: value is out of range.");
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
        throw std::runtime_error("Unsupported YARP value type for key '" + key + "'.");
    }

    yarp::os::Bottle* list = value.asList();
    if (list == nullptr)
    {
        throw std::runtime_error("Invalid YARP list value for key '" + key + "'.");
    }

    if (list->size() == 0)
    {
        output.put(key, std::vector<std::string>{});
        return;
    }

    bool allInt = true;
    bool allFloat = true;
    bool allNumeric = true;
    bool allString = true;
    bool allBool = true;
    for (int i = 0; i < list->size(); ++i)
    {
        const yarp::os::Value& element = list->get(i);
        const bool isInt = element.isInt32() || element.isInt64();
        const bool isNumeric = isInt || element.isFloat64();
        allInt = allInt && isInt;
        allFloat = allFloat && element.isFloat64();
        allNumeric = allNumeric && isNumeric;
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
                    throw std::runtime_error("Cannot convert int64 list element for key '" + key
                                             + "' to dinrail int: value is out of range.");
                }

                converted.push_back(static_cast<int>(yarpValue));
            } else
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

    if (allNumeric)
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

    throw std::runtime_error("Unsupported mixed/unknown list type for key '" + key + "'.");
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

void YarpPropertyConverter::convertRecursive(const Parameters& dinrailParams,
                                             yarp::os::Property& yarpProp)
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

        if (dinrailParams.check<std::chrono::nanoseconds>(key))
        {
            // BLF-compatible duration encoding for YARP properties/files:
            // durations are serialized as strings in the format HH:MM:SS:ffffff.
            // This matches BLF YarpImplementation::setParameter(std::chrono::nanoseconds),
            // which delegates to durationToString():
            // https://github.com/gbionics/bipedal-locomotion-framework/commit/9d0ef06c665d4921289df754c0cc7c0bc8329817
            const auto& duration = dinrailParams.find(key).as<std::chrono::nanoseconds>();
            yarpProp.put(key, durationToBipedalYarpString(duration));
            continue;
        }

        if (dinrailParams.check<std::vector<bool>>(key))
        {
            yarp::os::Value* listValue = yarp::os::Value::makeList();
            yarp::os::Bottle* list = listValue->asList();
            if (list == nullptr)
            {
                delete listValue;
                throw std::runtime_error("Failed to create YARP list for key '" + key + "'.");
            }

            const auto& values = dinrailParams.find(key).as<std::vector<bool>>();
            for (const bool value : values)
            {
                // Keep BLF-compatible bool list encoding.
                // We intentionally serialize booleans using makeValue("true"/"false")
                // to match BLF YarpImplementation::setParameterPrivate<std::vector<bool>>
                // and YARP boolean list handling.
                list->add(yarp::os::Value::makeValue(value ? "true" : "false"));
            }

            yarpProp.put(key, listValue);
            continue;
        }

        if (dinrailParams.check<std::vector<int>>(key))
        {
            yarp::os::Value* listValue = yarp::os::Value::makeList();
            yarp::os::Bottle* list = listValue->asList();
            if (list == nullptr)
            {
                delete listValue;
                throw std::runtime_error("Failed to create YARP list for key '" + key + "'.");
            }

            const auto& values = dinrailParams.find(key).as<std::vector<int>>();
            for (const int value : values)
            {
                list->addInt32(value);
            }

            yarpProp.put(key, listValue);
            continue;
        }

        if (dinrailParams.check<std::vector<double>>(key))
        {
            yarp::os::Value* listValue = yarp::os::Value::makeList();
            yarp::os::Bottle* list = listValue->asList();
            if (list == nullptr)
            {
                delete listValue;
                throw std::runtime_error("Failed to create YARP list for key '" + key + "'.");
            }

            const auto& values = dinrailParams.find(key).as<std::vector<double>>();
            for (const double value : values)
            {
                list->addFloat64(value);
            }

            yarpProp.put(key, listValue);
            continue;
        }

        if (dinrailParams.check<std::vector<std::string>>(key))
        {
            yarp::os::Value* listValue = yarp::os::Value::makeList();
            yarp::os::Bottle* list = listValue->asList();
            if (list == nullptr)
            {
                delete listValue;
                throw std::runtime_error("Failed to create YARP list for key '" + key + "'.");
            }

            const auto& values = dinrailParams.find(key).as<std::vector<std::string>>();
            for (const auto& value : values)
            {
                list->addString(value);
            }

            yarpProp.put(key, listValue);
            continue;
        }

        if (dinrailParams.check<std::vector<std::chrono::nanoseconds>>(key))
        {
            yarp::os::Value* listValue = yarp::os::Value::makeList();
            yarp::os::Bottle* list = listValue->asList();
            if (list == nullptr)
            {
                delete listValue;
                throw std::runtime_error("Failed to create YARP list for key '" + key + "'.");
            }

            const auto& values
                = dinrailParams.find(key).as<std::vector<std::chrono::nanoseconds>>();
            for (const auto value : values)
            {
                // BLF-compatible duration vector encoding:
                // each entry is stored as the duration string format HH:MM:SS:ffffff.
                list->addString(durationToBipedalYarpString(value));
            }

            yarpProp.put(key, listValue);
            continue;
        }

        if (dinrailParams.check<std::string>(key))
        {
            yarpProp.put(key, dinrailParams.find(key).as<std::string>());
            continue;
        }

        throw std::runtime_error("Unsupported dinrail::Parameters value type for key '" + key
                                 + "'.");
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
