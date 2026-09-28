// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IINTERFACEADAPTER_H
#define DINRAIL_IINTERFACEADAPTER_H

namespace dinrail
{

/**
 * @brief Owns an interface exposed through an interop adapter.
 *
 * Interop plugins return instances of this class when they can adapt an
 * interface implemented by a device into the interface requested through
 * Device::view(). The adapter object, and therefore the returned interface,
 * remains alive until the device is closed or reopened.
 */
class IInterfaceAdapter
{
public:
    virtual ~IInterfaceAdapter();

    /**
     * @brief Retrieve the adapted interface owned by this object.
     * @return The adapted interface, or nullptr if adaptation failed.
     */
    virtual void* getInterface() = 0;
};

} // namespace dinrail

#endif // DINRAIL_IINTERFACEADAPTER_H
