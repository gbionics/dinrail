// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/RuntimeContext.h>

#include <dinrail/IDevice.h>
#include <dinrail/IInteropPlugin.h>
#include <dinrail/Parameters.h>
#include <dinrail/PluginUtils.h>

#include <sharedlibpp/SharedLibraryClassFactory.h>

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace dinrail
{

namespace
{

// Build a unique cache key from a library name and a factory symbol name.
// A newline is used as separator because neither component can contain one.
std::string pluginKey(const std::string& libraryName, const std::string& factoryName)
{
    return libraryName + '\n' + factoryName;
}

template <class T>
FactoryUniquePtr<T> make_factory_unique(const sharedlibpp::SharedLibraryClassFactory<T>& factory)
{
    auto* factory_ptr = &factory;
    FactoryDeleter<T> deleter;
    deleter.destroy_fn = [factory_ptr](T* p) {
        if (factory_ptr != nullptr)
        {
            factory_ptr->destroy(p);
        }
    };
    return FactoryUniquePtr<T>(factory.create(), std::move(deleter));
}

template <class T> FactoryUniquePtr<T> make_factory_unique_with_delete(std::unique_ptr<T>&& ptr)
{
    FactoryDeleter<T> deleter;
    deleter.destroy_fn = [](T* p) { delete p; };
    return FactoryUniquePtr<T>(ptr.release(), std::move(deleter));
}

} // namespace

struct RuntimeContext::Impl
{
    using DeviceFactory = sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>;
    using InteropFactory = sharedlibpp::SharedLibraryClassFactory<dinrail::IInteropPlugin>;

    // DevicePlugin bundles a factory with its own mutex so that concurrent
    // allocate() calls for different plugins do not block each other (only
    // calls on the *same* plugin are serialised). A plain value in the map
    // would not work because std::mutex is not movable, hence the
    // shared_ptr wrapping in devicePlugins below.
    struct DevicePlugin
    {
        std::unique_ptr<DeviceFactory> factory;
        std::mutex factoryCreateMutex; // protects concurrent factory->create() calls for a specific
                                       // plugin

        FactoryUniquePtr<IDevice> allocate()
        {
            std::lock_guard<std::mutex> lock(factoryCreateMutex);
            return make_factory_unique<IDevice>(*factory);
        }
    };

    struct InteropPlugin
    {
        std::unique_ptr<InteropFactory> factory;
        std::mutex factoryCreateMutex; // protects concurrent factory->create() calls for a specific
                                       // plugin

        FactoryUniquePtr<IInteropPlugin> allocate()
        {
            std::lock_guard<std::mutex> lock(factoryCreateMutex);
            return make_factory_unique<IInteropPlugin>(*factory);
        }
    };

    Impl() = default;

    std::shared_ptr<DevicePlugin>
    getDevicePlugin(const std::string& libraryName, const std::string& factoryName)
    {
        const std::string key = pluginKey(libraryName, factoryName);

        // First lookup: check if already cached
        {
            std::lock_guard<std::mutex> lock(devicePluginsCacheMutex);
            auto existing = devicePlugins.find(key);
            if (existing != devicePlugins.end())
            {
                return existing->second;
            }
        }

        // Load plugin outside the lock to allow concurrent loads of different plugins
        auto plugin = std::make_shared<DevicePlugin>();
        plugin->factory = std::make_unique<DeviceFactory>(SHLIBPP_DEFAULT_START_CHECK,
                                                          SHLIBPP_DEFAULT_END_CHECK,
                                                          SHLIBPP_DEFAULT_SYSTEM_VERSION,
                                                          factoryName.c_str());

        // Extend search path with all configured plugin directories (dinrail lib dir +
        // DINRAIL_PLUGIN_PATH)
        const auto pluginSearchPaths = getPluginSearchPaths();
        for (const auto& path : pluginSearchPaths)
        {
            plugin->factory->extendSearchPath(path.string());
        }

        bool ok = plugin->factory->open(libraryName.c_str(), factoryName.c_str());
        ok = ok && plugin->factory->isValid();
        if (!ok)
        {
            return nullptr;
        }

        // Second lookup and insert: re-check cache in case another thread loaded it first
        {
            std::lock_guard<std::mutex> lock(devicePluginsCacheMutex);
            auto existing = devicePlugins.find(key);
            if (existing != devicePlugins.end())
            {
                // Another thread loaded it first, return theirs
                return existing->second;
            }
            // Only cache successful loads; failures are not cached so that a
            // subsequent open() call can retry (e.g. after the library becomes
            // available on the search path).
            devicePlugins.emplace(key, plugin);
        }

        return plugin;
    }

    std::shared_ptr<InteropPlugin>
    getInteropPlugin(const std::string& libraryName, const std::string& factoryName)
    {
        const std::string key = pluginKey(libraryName, factoryName);

        // First lookup: check if already cached
        {
            std::lock_guard<std::mutex> lock(interopPluginsCacheMutex);
            auto existing = interopPlugins.find(key);
            if (existing != interopPlugins.end())
            {
                return existing->second;
            }
        }

        // Load plugin outside the lock to allow concurrent loads of different plugins
        auto plugin = std::make_shared<InteropPlugin>();
        plugin->factory = std::make_unique<InteropFactory>(SHLIBPP_DEFAULT_START_CHECK,
                                                           SHLIBPP_DEFAULT_END_CHECK,
                                                           SHLIBPP_DEFAULT_SYSTEM_VERSION,
                                                           factoryName.c_str());

        // Extend search path with all configured plugin directories (dinrail lib dir +
        // DINRAIL_PLUGIN_PATH)
        const auto pluginSearchPaths = getPluginSearchPaths();
        for (const auto& path : pluginSearchPaths)
        {
            plugin->factory->extendSearchPath(path.string());
        }

        bool ok = plugin->factory->open(libraryName.c_str(), factoryName.c_str());
        ok = ok && plugin->factory->isValid();
        if (!ok)
        {
            return nullptr;
        }

        // Second lookup and insert: re-check cache in case another thread loaded it first
        {
            std::lock_guard<std::mutex> lock(interopPluginsCacheMutex);
            auto existing = interopPlugins.find(key);
            if (existing != interopPlugins.end())
            {
                // Another thread loaded it first, return theirs
                return existing->second;
            }
            // Only cache successful loads; failures are not cached so that a
            // subsequent open() call can retry (e.g. after the library becomes
            // available on the search path).
            interopPlugins.emplace(key, plugin);
        }

        return plugin;
    }

    FactoryUniquePtr<IDevice> createDevice(const Parameters& config)
    {
        if (!config.check<std::string>("device"))
        {
            std::cerr << "dinrail::Device: missing required parameter 'device'" << std::endl;
            return nullptr;
        }

        const std::string deviceName = config.find("device").as<std::string>();

        const std::string libraryName = getSharedlibppLibraryNameFromDeviceName(deviceName);
        const std::string factoryName = getSharedlibppFactoryNameFromDeviceName(deviceName);
        const std::string libraryLocation
            = findNativeDevicePluginLibrary(deviceName).value_or(libraryName);

        // Prefer an exact candidate path. This is required for MODULE libraries
        // on macOS, which use .so rather than .dylib.
        auto plugin = getDevicePlugin(libraryLocation, factoryName);
        std::string nativeFailureDiagnostic;

        if (plugin)
        {
            auto driver = plugin->allocate();
            if (driver && driver->open(config))
            {
                return driver;
            }
            if (!driver)
            {
                nativeFailureDiagnostic = "dinrail::Device: impossible to create instance for "
                                          "device '"
                                          + deviceName + "' from library '" + libraryName + "'";
            } else
            {
                nativeFailureDiagnostic = "dinrail::Device: device '" + deviceName
                                          + "' failed to open with provided config";
            }
        } else
        {
            nativeFailureDiagnostic = "dinrail::Device: impossible to find library '" + libraryName
                                      + "' for device '" + deviceName + "' (factory symbol: '"
                                      + factoryName + "')";
        }

        for (const auto& interopPluginInfo : getAvailableInteropPlugins())
        {
            const auto& interopName = interopPluginInfo.name;
            const std::string interopFactoryName
                = getSharedlibppFactoryNameFromInteropName(interopName);
            auto interopPlugin = getInteropPlugin(interopPluginInfo.location, interopFactoryName);
            if (!interopPlugin)
            {
                continue;
            }

            auto interop = interopPlugin->allocate();
            if (!interop)
            {
                std::cerr << "dinrail::Device: impossible to create instance for interop plugin '"
                          << interopName << "' from library '" << interopPluginInfo.location << "'"
                          << std::endl;
                continue;
            }

            auto interopDriver = interop->createDevice(config);
            if (interopDriver)
            {
                return make_factory_unique_with_delete<IDevice>(std::move(interopDriver));
            }
        }

        if (!nativeFailureDiagnostic.empty())
        {
            std::cerr << nativeFailureDiagnostic << std::endl;
        }

        return nullptr;
    }

    std::mutex devicePluginsCacheMutex; // protects devicePlugins map insertions and lookups
    std::unordered_map<std::string, std::shared_ptr<DevicePlugin>> devicePlugins;

    std::mutex interopPluginsCacheMutex; // protects interopPlugins map insertions and lookups
    std::unordered_map<std::string, std::shared_ptr<InteropPlugin>> interopPlugins;
};

RuntimeContext::RuntimeContext()
    : m_pimpl(std::make_shared<Impl>())
{
}

RuntimeContext::~RuntimeContext() = default;

const RuntimeContext& RuntimeContext::getDefault()
{
    static RuntimeContext context;
    return context;
}

FactoryUniquePtr<IDevice> RuntimeContext::createDevice(const Parameters& config)
{
    return m_pimpl->createDevice(config);
}

std::vector<dinrail::InteropDevices> RuntimeContext::listInteropDevices() const
{
    return getAvailableInteropDevices();
}

std::vector<dinrail::DeviceInfo> RuntimeContext::listNativeDevices() const
{
    return getAvailableNativeDevices();
}

std::vector<dinrail::InteropPluginInfo> RuntimeContext::listInteropPlugins() const
{
    return getAvailableInteropPlugins();
}

dinrail::AvailableDevices RuntimeContext::listDevices() const
{
    return getAvailableDevices();
}

} // namespace dinrail
