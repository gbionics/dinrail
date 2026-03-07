// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IDevice.h>
#include <dinrail/ICompatibilityLayer.h>
#include <dinrail/Constants.h>

#include <sharedlibpp/SharedLibraryClassFactory.h>
#include <sharedlibpp/SharedLibraryClass.h>

#include <optional>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace dinrail
{

// Get the path of the dinrail shared library to extend plugin search path
static std::optional<std::string> getPathOfDinrailSharedLibrary()
{
    std::error_code fs_error;

    // Get location of the library
    std::filesystem::path library_location;
#ifndef _WIN32
    Dl_info address_info;
    int res_val = dladdr(reinterpret_cast<void*>(getPathOfDinrailSharedLibrary), &address_info);
    if (address_info.dli_fname && res_val > 0)
    {
        library_location = address_info.dli_fname;
    }
    else
    {
        return {};
    }
#else
    char module_path[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&getPathOfDinrailSharedLibrary,
                          &hm) == 0)
    {
        return {};
    }
    if (GetModuleFileNameA(hm, module_path, sizeof(module_path)) == 0)
    {
        return {};
    }

    library_location = std::string(module_path);
#endif

    const std::filesystem::path library_directory = library_location.parent_path();
    return library_directory.string();
}

struct Device::Impl
{
    bool isValid{false};
    std::unique_ptr<dinrail::IDevice> driver;
    std::unique_ptr<sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>> driverFactory;

    // List of available compatibility layers to try
    std::vector<std::string> compatibilityLayers{"yarp"};

    Impl()
        : isValid{false}
        , driver{nullptr}
    {
    }

    // Helper function to try loading a compatibility layer
    std::unique_ptr<dinrail::IDevice> tryCompatibilityLayer(const std::string& compatName,
                                                            const Parameters& config,
                                                            const std::optional<std::string>& searchPath)
    {
        std::string libraryName = getSharedlibppLibraryNameFromCompatName(compatName);
        std::string factoryName = getSharedlibppFactoryNameFromCompatName(compatName);

        auto compatFactory = std::make_unique<sharedlibpp::SharedLibraryClassFactory<dinrail::ICompatibilityLayer>>(
            SHLIBPP_DEFAULT_START_CHECK,
            SHLIBPP_DEFAULT_END_CHECK,
            SHLIBPP_DEFAULT_SYSTEM_VERSION,
            factoryName.c_str());

        if (searchPath.has_value())
        {
            compatFactory->extendSearchPath(searchPath.value());
        }

        bool ok = compatFactory->open(libraryName.c_str(), factoryName.c_str());
        ok = ok && compatFactory->isValid();

        if (!ok)
        {
            return nullptr;
        }

        sharedlibpp::SharedLibraryClass<dinrail::ICompatibilityLayer> compatLayer(*compatFactory);
        std::unique_ptr<dinrail::ICompatibilityLayer> layerInstance(compatLayer.getContent().allocateInstance());

        if (!layerInstance)
        {
            return nullptr;
        }

        // Register interface adapters for this compatibility layer
        layerInstance->registerInterfaceAdapters();

        // Try to create device through compatibility layer
        return layerInstance->createDevice(config);
    }
};

Device::Device()
    : m_pimpl{std::make_unique<Impl>()}
{
}

Device::~Device()
{
    // Ensure that the driver is deallocated before the factory
    m_pimpl->driver.reset();
    m_pimpl->driverFactory.reset();
}

bool Device::open(const Parameters& config)
{
    // Ensure that the driver is deallocated before the factory
    m_pimpl->driver.reset();
    m_pimpl->isValid = false;

    // Extract device name from config
    if (!config.check<std::string>("device"))
    {
        // TODO: Add proper error logging
        return false;
    }

    const std::string deviceName = config.getString("device").value();

    // Check for device type preference
    std::string deviceType = "auto"; // default: try dinrail first, then fallback to compatibility layers
    if (config.check<std::string>("dinrail_device_type"))
    {
        const std::string configuredType = config.getString("dinrail_device_type").value();
        deviceType = configuredType;
    }

    // Get the expected library name and factory name given the device name
    std::string libraryName = getSharedlibppLibraryNameFromDeviceName(deviceName);
    std::string factoryName = getSharedlibppFactoryNameFromDeviceName(deviceName);

    std::optional<std::string> pathOfDinrailSharedLib = getPathOfDinrailSharedLibrary();

    // If deviceType is "yarp", skip trying to load dinrail device
    bool tryDinrailDevice = (deviceType != "yarp");
    bool tryCompatibilityLayers = (deviceType != "dinrail");

    if (tryDinrailDevice)
    {
        m_pimpl->driverFactory = std::make_unique<sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>>(
            SHLIBPP_DEFAULT_START_CHECK,
            SHLIBPP_DEFAULT_END_CHECK,
            SHLIBPP_DEFAULT_SYSTEM_VERSION,
            factoryName.c_str());

        // Extend the search path of the plugins to include the install prefix of the library
        if (pathOfDinrailSharedLib.has_value())
        {
            m_pimpl->driverFactory->extendSearchPath(pathOfDinrailSharedLib.value());
        }

        bool ok = m_pimpl->driverFactory->open(libraryName.c_str(), factoryName.c_str());
        ok = ok && m_pimpl->driverFactory->isValid();

        if (ok)
        {
            sharedlibpp::SharedLibraryClass<dinrail::IDevice> driver(*(m_pimpl->driverFactory));
            m_pimpl->driver.reset(driver.getContent().allocateInstance());

            // Open the device driver with the provided config
            if (m_pimpl->driver->open(config))
            {
                m_pimpl->isValid = true;
                return true;
            }
            
            // Failed to open dinrail device
            m_pimpl->driver.reset();
        }
        
        // If deviceType is explicitly "dinrail", don't try compatibility layers
        if (!tryCompatibilityLayers)
        {
            return false;
        }
    }

    // Device plugin not found or failed to open, try compatibility layers
    if (tryCompatibilityLayers)
    {
        for (const auto& compatName : m_pimpl->compatibilityLayers)
        {
            std::unique_ptr<dinrail::IDevice> compatDevice = 
                m_pimpl->tryCompatibilityLayer(compatName, config, pathOfDinrailSharedLib);
            
            if (compatDevice)
            {
                // Successfully created device through compatibility layer
                m_pimpl->driver = std::move(compatDevice);
                m_pimpl->isValid = true;
                return true;
            }
        }
    }
    
    // No device plugin or compatibility layer worked
    return false;
}

bool Device::close()
{
    if (m_pimpl->driver)
    {
        bool result = m_pimpl->driver->close();
        m_pimpl->isValid = false;
        return result;
    }
    m_pimpl->isValid = false;
    return false;
}

bool Device::isValid() const
{
    return m_pimpl->isValid;
}

IDevice* Device::getImplementation()
{
    return m_pimpl->driver.get();
}

std::string Device::getDeviceName() const
{
    if (m_pimpl->driver)
    {
        return "loaded device"; // IDevice doesn't have getDeviceName method
    }
    return "null";
}

} // namespace dinrail
