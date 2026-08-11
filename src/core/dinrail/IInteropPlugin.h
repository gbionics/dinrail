// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IINTEROPPLUGIN_H
#define DINRAIL_IINTEROPPLUGIN_H

#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>
#include <dinrail/PluginUtils.h>

#include <memory>
#include <string>
#include <vector>

namespace dinrail
{

/**
 * @brief Interface implemented by interop plugins.
 *
 * When a native dinrail device plugin cannot be opened, the available interop
 * plugins are asked, in alphabetical order, to open the device.
 */
class IInteropPlugin
{
public:
    virtual ~IInteropPlugin();

    /**
     * @brief Try to create and open a dinrail::IDevice from @p config.
     *
     * The returned device must implement dinrail::IInterfaceView to expose
     * the interfaces of the wrapped device to dinrail::Device::view().
     *
     * @param config Device configuration.
     * @return Opened device instance on success, nullptr otherwise.
     */
    virtual std::unique_ptr<dinrail::IDevice> createDevice(const Parameters& config) = 0;

    /**
     * @brief List the devices this interop plugin can open.
     *
     * Used for discovery and inspection (see `dinrail dev --list`). Plugins that
     * cannot enumerate their devices may return an empty list.
     *
     * @return Available devices with their name and location.
     */
    virtual std::vector<DeviceInfo> listDevices() const = 0;
};

} // namespace dinrail

#endif // DINRAIL_IINTEROPPLUGIN_H
