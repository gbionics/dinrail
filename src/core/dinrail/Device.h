// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DEVICE_H
#define DINRAIL_DEVICE_H

#include <dinrail/AdapterRegistry.h>
#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>

#include <memory>
#include <string>
#include <typeinfo>

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
     * @brief Destructor (required for pimpl).
     */
    ~Device();

    // Disable copy
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // Enable move
    Device(Device&&) = default;
    Device& operator=(Device&&) = default;

    /**
     * Open the Device by loading the device driver plugin.
     * The device name is extracted from the "device" parameter in config.
     * @param config is a list of parameters for the device.
     * @return true/false upon success/failure
     */
    bool open(const Parameters& config);

    /**
     * Close the Device.
     * @return true/false on success/failure.
     */
    bool close();

    /**
     * Check if device is valid.
     * @return true iff the device was created and configured successfully
     */
    bool isValid() const;

    /**
     * Get an interface to the device driver.
     *
     * @param x A pointer of type T which will be set to point to this
     * object if that is possible.
     *
     * @return true iff the desired interface is implemented by
     * the device driver.
     */
    template <class T> bool view(T*& x)
    {
        x = nullptr;

        IDevice* impl = getImplementation();
        if (!impl)
        {
            return false;
        }

        // First try adapter registry (for composition-based devices from compat layers)
        void* ptr = AdapterRegistry::queryAdapter(impl, typeid(T));
        if (ptr != nullptr)
        {
            x = static_cast<T*>(ptr);
            return true;
        }

        // Fallback to dynamic_cast for devices using inheritance
        T* v = dynamic_cast<T*>(impl);

        if (v != nullptr)
        {
            x = v;
            return true;
        }

        return false;
    }

    /**
     * Get the underlying device driver implementation.
     *
     * @return pointer to device driver
     */
    IDevice* getImplementation();

    /**
     * Get the name of the loaded device.
     *
     * @return device name, or "null" if no device is loaded
     */
    std::string getDeviceName() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;
};

} // namespace dinrail

#endif // DINRAIL_DEVICE_H
