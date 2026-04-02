// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IDEVICE_H
#define DINRAIL_IDEVICE_H

#include <dinrail/Parameters.h>

namespace dinrail
{

/**
 * @brief Interface implemented by all dinrail devices.
 *
 * This is the interface for any class that is a dinrail device.
 */
class IDevice
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IDevice() = default;

    /**
     * Open the IDevice.
     * @param config is a list of parameters for the device.
     * @return true/false upon success/failure
     */
    virtual bool open(const Parameters& config) = 0;

    /**
     * Close the DeviceDriver.
     * @return true/false on success/failure.
     */
    virtual bool close() = 0;

    /**
     * Allocate a new instance of the device driver.
     * This is used by the plugin system to create instances.
     * @return pointer to new instance
     */
    virtual IDevice* allocateInstance() const = 0;
};

} // namespace dinrail

#endif // DINRAIL_IDEVICE_H
