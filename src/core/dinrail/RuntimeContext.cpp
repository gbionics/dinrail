// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/RuntimeContext.h>

#include <dinrail/InterfaceAdapterRegistry.h>
#include <dinrail/ICompatibilityLayer.h>
#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>
#include <dinrail/PluginUtils.h>

#include <sharedlibpp/SharedLibraryClass.h>
#include <sharedlibpp/SharedLibraryClassFactory.h>

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace dinrail
{

namespace
{

std::string pluginKey(const std::string& libraryName, const std::string& factoryName)
{
    return libraryName + '\n' + factoryName;
}

} // namespace

struct RuntimeContext::Impl
{
    using DeviceFactory = sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>;
    using CompatibilityFactory
        = sharedlibpp::SharedLibraryClassFactory<dinrail::ICompatibilityLayer>;

    struct DevicePlugin
    {
        std::unique_ptr<DeviceFactory> factory;
        std::mutex mutex;

        std::unique_ptr<IDevice> allocate()
        {
            std::lock_guard<std::mutex> lock(mutex);
            return std::unique_ptr<IDevice>(factory->create());
        }
    };

    struct CompatibilityPlugin
    {
        std::unique_ptr<CompatibilityFactory> factory;
        std::mutex mutex;

        std::unique_ptr<ICompatibilityLayer> allocate()
        {
            std::lock_guard<std::mutex> lock(mutex);
            return std::unique_ptr<ICompatibilityLayer>(factory->create());
        }
    };

    Impl()
        : searchPath(getPathOfDinrailSharedLibrary())
    {
    }

    std::shared_ptr<DevicePlugin> getDevicePlugin(const std::string& libraryName,
                                                   const std::string& factoryName)
    {
        const std::string key = pluginKey(libraryName, factoryName);
        std::lock_guard<std::mutex> lock(pluginMutex);

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
            // Cache misses as well as successful loads. A Context represents a
            // stable plug-in environment and should not probe the filesystem
            // again for every Device opened in automatic mode.
            devicePlugins.emplace(key, nullptr);
            return nullptr;
        }

        devicePlugins.emplace(key, plugin);
        return plugin;
    }

    std::shared_ptr<CompatibilityPlugin>
    getCompatibilityPlugin(const std::string& libraryName, const std::string& factoryName)
    {
        const std::string key = pluginKey(libraryName, factoryName);
        std::lock_guard<std::mutex> lock(pluginMutex);

        auto existing = compatibilityPlugins.find(key);
        if (existing != compatibilityPlugins.end())
        {
            return existing->second;
        }

        auto plugin = std::make_shared<CompatibilityPlugin>();
        plugin->factory = std::make_unique<CompatibilityFactory>(SHLIBPP_DEFAULT_START_CHECK,
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
            compatibilityPlugins.emplace(key, nullptr);
            return nullptr;
        }

        // Registration is context-local and happens only the first time this
        // compatibility plug-in is loaded into this Context.
        auto layer = plugin->allocate();
        if (!layer)
        {
            compatibilityPlugins.emplace(key, nullptr);
            return nullptr;
        }
        layer->registerInterfaceAdapters(adapterRegistry);

        compatibilityPlugins.emplace(key, plugin);
        return plugin;
    }

    std::unique_ptr<IDevice> createDevice(const Parameters& config)
    {
        if (!config.check<std::string>("device"))
        {
            return nullptr;
        }

        const std::string deviceName = config.find("device").as<std::string>();

        std::string deviceType = "auto";
        if (config.check<std::string>("dinrail_device_type"))
        {
            deviceType = config.find("dinrail_device_type").as<std::string>();
        }

        const bool tryDinrailDevice = (deviceType != "yarp");
        const bool tryCompatibilityLayers = (deviceType != "dinrail");

        if (tryDinrailDevice)
        {
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

            if (!tryCompatibilityLayers)
            {
                return nullptr;
            }
        }

        for (const auto& compatName : compatibilityLayers)
        {
            const std::string libraryName = getSharedlibppLibraryNameFromCompatName(compatName);
            const std::string factoryName = getSharedlibppFactoryNameFromCompatName(compatName);
            auto plugin = getCompatibilityPlugin(libraryName, factoryName);
            if (!plugin)
            {
                continue;
            }

            auto layer = plugin->allocate();
            if (!layer)
            {
                continue;
            }

            auto driver = layer->createDevice(config);
            if (driver)
            {
                return driver;
            }
        }

        return nullptr;
    }

    std::mutex pluginMutex;
    std::unordered_map<std::string, std::shared_ptr<DevicePlugin>> devicePlugins;
    std::unordered_map<std::string, std::shared_ptr<CompatibilityPlugin>> compatibilityPlugins;
    std::vector<std::string> compatibilityLayers{"yarp"};
    std::optional<std::string> searchPath;

    // Keep this member last so it is destroyed before plug-in factories are
    // released. Adapter factories may contain code from those plug-ins.
    InterfaceAdapterRegistry adapterRegistry;
};

RuntimeContext::RuntimeContext()
    : m_pimpl(std::make_shared<Impl>())
{
}

RuntimeContext::~RuntimeContext() = default;

const RuntimeContext& RuntimeContext::get_default()
{
    static RuntimeContext context;
    return context;
}

std::unique_ptr<IDevice> RuntimeContext::createDevice(const Parameters& config)
{
    return m_pimpl->createDevice(config);
}

void* RuntimeContext::queryAdapter(IDevice* device, const std::type_info& interfaceType) const
{
    return m_pimpl->adapterRegistry.queryAdapter(device, interfaceType);
}

} // namespace dinrail
