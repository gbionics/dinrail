// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IINTERFACETRANSLATION_H
#define DINRAIL_IINTERFACETRANSLATION_H

namespace dinrail
{

/**
 * @brief Owns an interface exposed through an interop translation.
 *
 * Interop plugins return instances of this class when they can translate an
 * interface implemented by a device into the interface requested through
 * Device::view(). The translation object, and therefore the returned interface,
 * remains alive until the device is closed or reopened.
 */
class IInterfaceTranslation
{
public:
    virtual ~IInterfaceTranslation();

    /**
     * @brief Retrieve the translated interface owned by this object.
     * @return The translated interface, or nullptr if translation failed.
     */
    virtual void* getInterface() = 0;
};

} // namespace dinrail

#endif // DINRAIL_IINTERFACETRANSLATION_H
