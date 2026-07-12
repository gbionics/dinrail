// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IINTERFACEQUERYABLE_H
#define DINRAIL_IINTERFACEQUERYABLE_H

#include <typeinfo>

namespace dinrail
{

/**
 * @brief Optional capability for devices that can resolve interfaces dynamically.
 */
class IInterfaceQueryable
{
public:
    virtual ~IInterfaceQueryable() = default;

    /**
     * Query an arbitrary interface implemented by the underlying device.
     * @param interfaceType Requested interface type information.
     * @return Pointer to the requested interface if available, nullptr otherwise.
     */
    virtual void* queryInterface(const std::type_info& interfaceType) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IINTERFACEQUERYABLE_H