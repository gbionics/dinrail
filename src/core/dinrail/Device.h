// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DEVICE_H
#define DINRAIL_DEVICE_H

#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>

#include <memory>
#include <string>

namespace dinrail
{

/**
 * @brief Runtime handle for opening and querying dinrail device plugins.
 */
class Device
{
public:
    /**
     * @brief Construct an empty device handle.
     */
    Device();

    /**
     * @brief Destroy the device handle and release owned resources.
     */
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&&) = default;
    Device& operator=(Device&&) = default;

    /**
     * @brief Open a device plugin from the provided configuration.
     * @param config Device configuration, including at least the device name.
     * @return true on success, false otherwise.
     */
    bool open(const Parameters& config);

    /**
     * @brief Close the currently open device, if any.
     * @return true on success, false otherwise.
     */
    bool close();

    /**
     * @brief Check whether a device is currently open and valid.
     * @return true if a valid device implementation is available.
     */
    bool isValid() const;

    /**
     * @brief Retrieve a typed interface implemented by the underlying device.
     * @tparam T Interface type to query.
     * @param x Output pointer receiving the requested interface on success.
     * @return true if the interface is available, false otherwise.
     */
    template <class T> bool view(T*& x)
    {
        x = nullptr;

        IDevice* impl = getImplementation();
        if (!impl)
        {
            return false;
        }

        T* v = dynamic_cast<T*>(impl);
        if (v != nullptr)
        {
            x = v;
            return true;
        }

        return false;
    }

private:
    struct Impl;
    std::unique_ptr<Impl> m_pimpl;

    // Internal method to retrieve the raw device implementation pointer,
    // used in the view() method for dynamic casting.
    IDevice* getImplementation();
};

} // namespace dinrail

#endif // DINRAIL_DEVICE_H
