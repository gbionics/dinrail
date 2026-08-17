// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IINTERFACETRANSLATIONPROVIDER_H
#define DINRAIL_IINTERFACETRANSLATIONPROVIDER_H

#include <dinrail/IInterfaceTranslation.h>

#include <memory>
#include <typeinfo>

namespace dinrail
{

class IDevice;

/**
 * @brief Optional capability for interop plugins that translate interfaces.
 *
 * Keeping translations in a separate capability lets existing interop plugins
 * continue to implement only IInteropPlugin.
 */
class IInterfaceTranslationProvider
{
public:
    virtual ~IInterfaceTranslationProvider();

    /**
     * @brief Try to translate an interface exposed by @p device.
     *
     * This method is called by dinrail::Device::view() after direct and dynamic
     * interface lookup have failed. Implementations can inspect the source
     * device (typically through dinrail::IInterfaceView), then return an owned
     * adapter implementing @p interfaceType.
     *
     * @param device Device whose interfaces can be used as translation sources.
     * @param interfaceType Requested destination interface type.
     * @return Translation object on success, nullptr otherwise.
     */
    virtual std::unique_ptr<IInterfaceTranslation>
    createInterfaceTranslation(IDevice& device, const std::type_info& interfaceType) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IINTERFACETRANSLATIONPROVIDER_H
