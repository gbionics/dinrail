// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IINTERFACEVIEW_H
#define DINRAIL_IINTERFACEVIEW_H

#include <typeinfo>

namespace dinrail
{

/**
 * @brief Optional capability for devices that can resolve interfaces dynamically.
 *
 * Devices that wrap a foreign implementation (for example, interop plugins)
 * implement this to let dinrail::Device::view() resolve interfaces that are not
 * reachable through a direct dynamic_cast on dinrail::IDevice.
 */
class IInterfaceView
{
public:
    virtual ~IInterfaceView();

    /**
     * @brief Resolve an interface implemented by the underlying device.
     * @param interfaceType Requested interface type information.
     * @return Pointer to the requested interface if available, nullptr otherwise.
     */
    virtual void* viewInterface(const std::type_info& interfaceType) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IINTERFACEVIEW_H
