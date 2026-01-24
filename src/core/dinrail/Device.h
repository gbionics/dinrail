// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DEVICE_H
#define DINRAIL_DEVICE_H

#include <dinrail/Property.h>

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

    /**
     * Open the Device.
     * @param config is a list of parameters for the device.
     * @return true/false upon success/failure
     */
    virtual bool open(const Property& config);

    /**
     * Close the Device.
     * @return true/false on success/failure.
     */
    virtual bool close();

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
    template <class T>
    bool view(T*& x)
    {
        x = nullptr;

        // This requires RTTI to be compiled in.
        T* v = dynamic_cast<T*>(getImplementation());

        if (v != nullptr)
        {
            x = v;
            return true;
        }

        return false;
    }

    /**
     * Some drivers are bureaucrats, pointing at others. Such drivers override
     * this method.
     *
     * @return "real" device driver
     */
    virtual Device* getImplementation();

private:
    bool m_isValid{false};
};

} // namespace dinrail

#endif // DINRAIL_DEVICE_H
