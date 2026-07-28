// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/RuntimeContext.h>

#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>
#include <dinrail/PluginUtils.h>

#include <sharedlibpp/SharedLibraryClass.h>
#include <sharedlibpp/SharedLibraryClassFactory.h>

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

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

} // namespace

struct RuntimeContext::Impl
{
    using DeviceFactory = sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>;

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

    Impl()
    {
    }

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
        auto plugin = getDevicePlugin(libraryName, factoryName);

        if (plugin)
        {
            auto driver = plugin->allocate();
            if (driver && driver->open(config))
            {
                return driver;
            }
            if (!driver)
            {
                std::cerr << "dinrail::Device: impossible to create instance for device '"
                          << deviceName << "' from library '" << libraryName << "'" << std::endl;
            } else
            {
                std::cerr << "dinrail::Device: device '" << deviceName
                          << "' failed to open with provided config" << std::endl;
            }
        } else
        {
            std::cerr << "dinrail::Device: impossible to find library '" << libraryName
                      << "' for device '" << deviceName << "' (factory symbol: '" << factoryName
                      << "')" << std::endl;
        }

        return nullptr;
    }

    std::mutex devicePluginsCacheMutex; // protects devicePlugins map insertions and lookups
    std::unordered_map<std::string, std::shared_ptr<DevicePlugin>> devicePlugins;
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

} // namespace dinrail
