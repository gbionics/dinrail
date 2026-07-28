// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/RuntimeContext.h>

#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>
#include <dinrail/PluginUtils.h>

#include <sharedlibpp/SharedLibraryClass.h>
#include <sharedlibpp/SharedLibraryClassFactory.h>

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

        std::unique_ptr<IDevice> allocate()
        {
            std::lock_guard<std::mutex> lock(factoryCreateMutex);
            return std::unique_ptr<IDevice>(factory->create());
        }
    };

    Impl()
        : searchPath(getPathOfDinrailSharedLibrary())
    {
    }

    std::shared_ptr<DevicePlugin>
    getDevicePlugin(const std::string& libraryName, const std::string& factoryName)
    {
        const std::string key = pluginKey(libraryName, factoryName);
        std::lock_guard<std::mutex> lock(devicePluginsCacheMutex);

        auto existing = devicePlugins.find(key);
        if (existing != devicePlugins.end())
        {
            return existing->second;
        }

        auto plugin = std::make_shared<DevicePlugin>();
        plugin->factory = std::make_unique<DeviceFactory>(SHLIBPP_DEFAULT_START_CHECK,
                                                          SHLIBPP_DEFAULT_END_CHECK,
                                                          SHLIBPP_DEFAULT_SYSTEM_VERSION,
                                                          factoryName.c_str());

        if (searchPath.has_value())
        {
            plugin->factory->extendSearchPath(searchPath.value());
        }

        bool ok = plugin->factory->open(libraryName.c_str(), factoryName.c_str());
        ok = ok && plugin->factory->isValid();
        if (!ok)
        {
            return nullptr;
        }

        // Only cache successful loads; failures are not cached so that a
        // subsequent open() call can retry (e.g. after the library becomes
        // available on the search path).
        devicePlugins.emplace(key, plugin);
        return plugin;
    }

    std::unique_ptr<IDevice> createDevice(const Parameters& config)
    {
        if (!config.check<std::string>("device"))
        {
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
        }

        return nullptr;
    }

    std::mutex devicePluginsCacheMutex; // protects devicePlugins map insertions and lookups
    std::unordered_map<std::string, std::shared_ptr<DevicePlugin>> devicePlugins;
    std::optional<std::string> searchPath;
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

std::unique_ptr<IDevice> RuntimeContext::createDevice(const Parameters& config)
{
    return m_pimpl->createDevice(config);
}

} // namespace dinrail
