// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "YarpPropertyConverter.h"

#include <yarp/os/Property.h>
#include <type_traits>
#include <variant>

namespace dinrail
{

yarp::os::Property YarpPropertyConverter::toYarpProperty(const Parameters& dinrailParams)
{
    yarp::os::Property yarpProp;
    convertRecursive(dinrailParams, yarpProp);
    return yarpProp;
}

void YarpPropertyConverter::convertRecursive(const Parameters& dinrailParams, yarp::os::Property& yarpProp)
{
    // First, copy all data (non-group) keys
    std::vector<std::string> dataKeys = dinrailParams.getValueKeys();
    for (const auto& key : dataKeys)
    {
        const auto value = dinrailParams.get(key);
        if (value)
        {
            // Visit the variant and put the appropriate type
            std::visit([&yarpProp, &key](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, bool>)
                {
                    yarpProp.put(key, arg ? 1 : 0);
                }
                else if constexpr (std::is_same_v<T, std::int64_t>)
                {
                    yarpProp.put(key, static_cast<int>(arg));
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    yarpProp.put(key, arg);
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    yarpProp.put(key, arg);
                }
            }, value->get());
        }
    }
    
    // Then, recursively convert all nested groups
    std::vector<std::string> groupKeys = dinrailParams.getGroupKeys();
    for (const auto& key : groupKeys)
    {
        const auto nestedGroup = dinrailParams.findGroup(key);
        if (nestedGroup)
        {
            yarp::os::Property& yarpNestedGroup = yarpProp.addGroup(key);
            convertRecursive(nestedGroup->get(), yarpNestedGroup);
        }
    }
}

} // namespace dinrail
