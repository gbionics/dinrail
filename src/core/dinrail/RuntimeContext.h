// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_RUNTIMECONTEXT_H
#define DINRAIL_RUNTIMECONTEXT_H

#include <dinrail/PluginUtils.h>

#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace dinrail
{

class Device;
class IDevice;
class IInterfaceAdapter;
class Parameters;

// Deleter for unique_ptr that calls a stored destroy function instead of delete.
// This is used to manage the lifetime of devices created via plugin factories.
template <class T> struct FactoryDeleter
{
    std::function<void(T*)> destroy_fn;

    void operator()(T* p) const noexcept
    {
        if (p != nullptr && destroy_fn)
        {
            destroy_fn(p);
        }
    }
};

template <class T> using FactoryUniquePtr = std::unique_ptr<T, FactoryDeleter<T>>;

/**
 * Shared runtime state used to load device and interop plug-ins.
 *
 * A RuntimeContext owns the plug-in cache. Each interop plug-in is instantiated
 * at most once and reused for all operations in the context. Copies are
 * lightweight handles to the same state, so passing a RuntimeContext to a
 * Device is safe even when the original RuntimeContext object goes out of scope.
 */
class RuntimeContext
{
public:
    RuntimeContext();
    ~RuntimeContext();

    RuntimeContext(const RuntimeContext&) = default;
    RuntimeContext& operator=(const RuntimeContext&) = default;
    RuntimeContext(RuntimeContext&&) noexcept = default;
    RuntimeContext& operator=(RuntimeContext&&) noexcept = default;

    /**
     * Process-wide context used by the simple constructors.
     */
    static const RuntimeContext& getDefault();

    /**
     * @brief List native dinrail device plugins discoverable on the search paths.
     *
     * @return Sorted, de-duplicated devices with name and plugin location.
     */
    std::vector<dinrail::DeviceInfo> listNativeDevices() const;

    /**
     * @brief List available interop plugins discoverable on the search paths.
     *
     * @return Sorted, de-duplicated interop plugins with name and plugin location.
     */
    std::vector<dinrail::InteropPluginInfo> listInteropPlugins() const;

    /**
     * @brief List the devices provided by the available interop plugins.
     *
     * Loads each interop plugin and queries `IInteropPlugin::listDevices()`.
     * Mainly intended for inspection (see `dinrail dev --list`).
     *
     * @return One entry per interop plugin, grouped by plugin name.
     */
    std::vector<dinrail::InteropDevices> listInteropDevices() const;

    /**
     * @brief List all discoverable devices, including interop-provided ones.
     *
     * @return Native devices and interop-provided devices grouped by interop plugin.
     */
    dinrail::AvailableDevices listDevices() const;

private:
    struct Impl;
    std::shared_ptr<Impl> m_pimpl;

    /**
     * @brief Load and open a device from the provided configuration.
     *
     * Looks up (or loads) the plug-in library for the device named by
     * `config["device"]`, creates an instance, and calls `IDevice::open()`.
     * `config["dinrail_device_type"]` selects the implementation: `auto`
     * (the default) tries the native plugin first and then every available
     * interop plugin, `dinrail` tries only the native plugin, and any other
     * value selects an interop plugin by name (for example, `yarp`).
     * Returns nullptr on any failure.
     */
    FactoryUniquePtr<IDevice> createDevice(const Parameters& config);

    /** Ask available interop plugins to adapt a device interface. */
    std::unique_ptr<IInterfaceAdapter>
    createInterfaceAdapter(IDevice& device, const std::type_info& interfaceType);

    friend class Device;
};

} // namespace dinrail

#endif // DINRAIL_RUNTIMECONTEXT_H
