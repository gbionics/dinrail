// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_CONSTANTS_H
#define DINRAIL_CONSTANTS_H

#include <string>

namespace dinrail
{

/**
 * Get the library name from the device name
 * @param deviceName the name of the device
 * @return the library name (e.g., "dinrail-fake-motor-control-board")
 */
inline std::string getSharedlibppLibraryNameFromDeviceName(const std::string& deviceName)
{
    return "dinrail-device-" + deviceName;
}

/**
 * Get the factory name from the device name
 * @param deviceName the name of the device
 * @return the factory name (e.g., "dinrail_fake_motor_control_board")
 */
inline std::string getSharedlibppFactoryNameFromDeviceName(const std::string& deviceName)
{
    // Replace hyphens with underscores for the factory name
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

/**
 * Get the library name from the compatibility layer name
 * @param compatName the name of the compatibility layer (e.g., "yarp")
 * @return the library name (e.g., "dinrail-compat-yarp")
 */
inline std::string getSharedlibppLibraryNameFromCompatName(const std::string& compatName)
{
    return "dinrail-compat-" + compatName;
}

/**
 * Get the factory name from the compatibility layer name
 * @param compatName the name of the compatibility layer (e.g., "yarp")
 * @return the factory name (e.g., "dinrail_compat_yarp")
 */
inline std::string getSharedlibppFactoryNameFromCompatName(const std::string& compatName)
{
    // Replace hyphens with underscores for the factory name
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
