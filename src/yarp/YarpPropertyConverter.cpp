// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "YarpPropertyConverter.h"

#include <yarp/os/Property.h>
#include <variant>

namespace dinrail
{

yarp::os::Property YarpPropertyConverter::toYarpProperty(const Property& dinrailProp)
{
    yarp::os::Property yarpProp;
    convertRecursive(dinrailProp, yarpProp);
    return yarpProp;
}

void YarpPropertyConverter::convertRecursive(const Property& dinrailProp, yarp::os::Property& yarpProp)
{
    // First, copy all data (non-group) keys
    std::vector<std::string> dataKeys = dinrailProp.getDataKeys();
    for (const auto& key : dataKeys)
    {
        const Property::Value* value = dinrailProp.getValue(key);
        if (value)
        {
            // Visit the variant and put the appropriate type
            std::visit([&yarpProp, &key](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, int>)
                {
                    yarpProp.put(key, arg);
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    yarpProp.put(key, arg);
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    yarpProp.put(key, arg);
                }
            }, *value);
        }
    }
    
    // Then, recursively convert all nested groups
    std::vector<std::string> groupKeys = dinrailProp.getGroupKeys();
    for (const auto& key : groupKeys)
    {
        const Property* nestedGroup = dinrailProp.findGroup(key);
        if (nestedGroup)
        {
            yarp::os::Property& yarpNestedGroup = yarpProp.addGroup(key);
            convertRecursive(*nestedGroup, yarpNestedGroup);
        }
    }
}

} // namespace dinrail
