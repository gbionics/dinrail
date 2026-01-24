// SPDX-FileCopyrightText: 2026 dinrail contributors
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DEVICE_H
#define DINRAIL_DEVICE_H

#include <string>

namespace dinrail
{

/**
 * @brief Factory class for instantiating a dinrail device.
 *
 * A Device represents a standalone piece of robot functionality that exposes
 * its capabilities through C++ interfaces.
 */
class Device
{
public:
    /**
     * @brief Default constructor.
     */
    Device();

    /**
     * @brief Virtual destructor.
     */
    virtual ~Device();

    // Disable copy
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // Enable move
    Device(Device&&) = default;
    Device& operator=(Device&&) = default;
};

} // namespace dinrail

#endif // DINRAIL_DEVICE_H
