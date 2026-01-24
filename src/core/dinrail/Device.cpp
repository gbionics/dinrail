// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IDevice.h>
#include <dinrail/Constants.h>

#include <sharedlibpp/SharedLibraryClassFactory.h>
#include <sharedlibpp/SharedLibraryClass.h>

#include <optional>
#include <filesystem>

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

    Impl()
        : isValid{false}
        , driver{nullptr}
    {
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

bool Device::open(const Property& config)
{
    // Ensure that the driver is deallocated before the factory
    m_pimpl->driver.reset();
    m_pimpl->isValid = false;

    // Extract device name from config
    if (!config.check("device"))
    {
        // TODO: Add proper error logging
        return false;
    }

    std::string deviceName = config.get("device");
    if (deviceName.empty())
    {
        return false;
    }

    // Get the expected library name and factory name given the device name
    std::string libraryName = getSharedlibppLibraryNameFromDeviceName(deviceName);
    std::string factoryName = getSharedlibppFactoryNameFromDeviceName(deviceName);

    m_pimpl->driverFactory = std::make_unique<sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>>(
        SHLIBPP_DEFAULT_START_CHECK,
        SHLIBPP_DEFAULT_END_CHECK,
        SHLIBPP_DEFAULT_SYSTEM_VERSION,
        factoryName.c_str());

    // Extend the search path of the plugins to include the install prefix of the library
    std::optional<std::string> pathOfDinrailSharedLib = getPathOfDinrailSharedLibrary();
    if (pathOfDinrailSharedLib.has_value())
    {
        m_pimpl->driverFactory->extendSearchPath(pathOfDinrailSharedLib.value());
    }

    bool ok = m_pimpl->driverFactory->open(libraryName.c_str(), factoryName.c_str());
    ok = ok && m_pimpl->driverFactory->isValid();

    if (!ok)
    {
        // TODO: Add proper error logging
        return false;
    }

    sharedlibpp::SharedLibraryClass<dinrail::IDevice> driver(*(m_pimpl->driverFactory));
    m_pimpl->driver.reset(driver.getContent().allocateInstance());

    // Open the device driver with the provided config
    if (!m_pimpl->driver->open(config))
    {
        m_pimpl->driver.reset();
        return false;
    }

    m_pimpl->isValid = true;
    return true;
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
