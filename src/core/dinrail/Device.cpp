// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Constants.h>
#include <dinrail/Device.h>
#include <dinrail/ICompatibilityLayer.h>
#include <dinrail/IDevice.h>

#include <sharedlibpp/SharedLibraryClass.h>
#include <sharedlibpp/SharedLibraryClassFactory.h>

#include <filesystem>
#include <optional>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace dinrail
{

static std::optional<std::string> getPathOfDinrailSharedLibrary()
{
    std::filesystem::path libraryLocation;
#ifndef _WIN32
    Dl_info addressInfo;
    int resVal = dladdr(reinterpret_cast<void*>(getPathOfDinrailSharedLibrary), &addressInfo);
    if (addressInfo.dli_fname && resVal > 0)
    {
        libraryLocation = addressInfo.dli_fname;
    } else
    {
        return {};
    }
#else
    char modulePath[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                              | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&getPathOfDinrailSharedLibrary,
                          &hm)
        == 0)
    {
        return {};
    }
    if (GetModuleFileNameA(hm, modulePath, sizeof(modulePath)) == 0)
    {
        return {};
    }

    libraryLocation = std::string(modulePath);
#endif

    return libraryLocation.parent_path().string();
}

struct Device::Impl
{
    bool isValid{false};
    std::unique_ptr<dinrail::IDevice> driver;
    std::unique_ptr<sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>> driverFactory;
    std::vector<std::string> compatibilityLayers{"yarp"};

    std::unique_ptr<dinrail::IDevice>
    tryCompatibilityLayer(const std::string& compatName,
                          const Parameters& config,
                          const std::optional<std::string>& searchPath)
    {
        const std::string libraryName = getSharedlibppLibraryNameFromCompatName(compatName);
        const std::string factoryName = getSharedlibppFactoryNameFromCompatName(compatName);

        auto compatFactory = std::make_unique<sharedlibpp::SharedLibraryClassFactory<
            dinrail::ICompatibilityLayer>>(SHLIBPP_DEFAULT_START_CHECK,
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
        std::unique_ptr<dinrail::ICompatibilityLayer> layerInstance(
            compatLayer.getContent().allocateInstance());

        if (!layerInstance)
        {
            return nullptr;
        }

        layerInstance->registerInterfaceAdapters();
        return layerInstance->createDevice(config);
    }
};

Device::Device()
    : m_pimpl{std::make_unique<Impl>()}
{
}

Device::~Device()
{
    m_pimpl->driver.reset();
    m_pimpl->driverFactory.reset();
}

bool Device::open(const Parameters& config)
{
    m_pimpl->driver.reset();
    m_pimpl->isValid = false;

    if (!config.check<std::string>("device"))
    {
        return false;
    }

    const std::string deviceName = config.find("device").as<std::string>();

    std::string deviceType = "auto";
    if (config.check<std::string>("dinrail_device_type"))
    {
        deviceType = config.find("dinrail_device_type").as<std::string>();
    }

    const std::string libraryName = getSharedlibppLibraryNameFromDeviceName(deviceName);
    const std::string factoryName = getSharedlibppFactoryNameFromDeviceName(deviceName);

    const std::optional<std::string> pathOfDinrailSharedLib = getPathOfDinrailSharedLibrary();

    const bool tryDinrailDevice = (deviceType != "yarp");
    const bool tryCompatibilityLayers = (deviceType != "dinrail");

    if (tryDinrailDevice)
    {
        m_pimpl->driverFactory = std::make_unique<
            sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>>(SHLIBPP_DEFAULT_START_CHECK,
                                                                       SHLIBPP_DEFAULT_END_CHECK,
                                                                       SHLIBPP_DEFAULT_SYSTEM_VERSION,
                                                                       factoryName.c_str());

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

            if (m_pimpl->driver && m_pimpl->driver->open(config))
            {
                m_pimpl->isValid = true;
                return true;
            }

            m_pimpl->driver.reset();
        }

        if (!tryCompatibilityLayers)
        {
            return false;
        }
    }

    if (tryCompatibilityLayers)
    {
        for (const auto& compatName : m_pimpl->compatibilityLayers)
        {
            std::unique_ptr<dinrail::IDevice> compatDevice
                = m_pimpl->tryCompatibilityLayer(compatName, config, pathOfDinrailSharedLib);

            if (compatDevice)
            {
                m_pimpl->driver = std::move(compatDevice);
                m_pimpl->isValid = true;
                return true;
            }
        }
    }

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
        return "loaded device";
    }
    return "null";
}

} // namespace dinrail
