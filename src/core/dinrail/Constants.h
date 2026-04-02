// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_CONSTANTS_H
#define DINRAIL_CONSTANTS_H

#include <string>

namespace dinrail
{

inline std::string getSharedlibppLibraryNameFromDeviceName(const std::string& deviceName)
{
    return "dinrail-device-" + deviceName;
}

inline std::string getSharedlibppFactoryNameFromDeviceName(const std::string& deviceName)
{
    std::string factoryName = "dinrail_device_" + deviceName;
    for (char& c : factoryName)
    {
        if (c == '-')
        {
            c = '_';
        }
    }
    return factoryName;
}

inline std::string getSharedlibppLibraryNameFromCompatName(const std::string& compatName)
{
    return "dinrail-compat-" + compatName;
}

inline std::string getSharedlibppFactoryNameFromCompatName(const std::string& compatName)
{
    std::string factoryName = "dinrail_compat_" + compatName;
    for (char& c : factoryName)
    {
        if (c == '-')
        {
            c = '_';
        }
    }
    return factoryName;
}

} // namespace dinrail

#endif // DINRAIL_CONSTANTS_H
