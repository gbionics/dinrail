// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IDevice.h>
#include <dinrail/PluginUtils.h>

#include <sharedlibpp/SharedLibraryClassFactory.h>

namespace dinrail
{

namespace
{

template <class T>
struct FactoryDeleter
{
    const sharedlibpp::SharedLibraryClassFactory<T>* factory = nullptr;

    void operator()(T* p) const noexcept
    {
        if (p != nullptr && factory != nullptr)
        {
            factory->destroy(p);
        }
    }
};

template <class T>
using FactoryUniquePtr = std::unique_ptr<T, FactoryDeleter<T>>;

template <class T>
FactoryUniquePtr<T> make_factory_unique(const sharedlibpp::SharedLibraryClassFactory<T>& factory)
{
    return FactoryUniquePtr<T>(factory.create(), FactoryDeleter<T>{&factory});
}

} // namespace

struct Device::Impl
{
    bool isValid{false};
    std::unique_ptr<sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>> deviceFactory;
    FactoryUniquePtr<dinrail::IDevice> device;
};

Device::Device()
    : m_pimpl{std::make_unique<Impl>()}
{
}

Device::~Device()
{
    close();
}

bool Device::open(const Parameters& config)
{
    m_pimpl->device.reset();
    m_pimpl->isValid = false;

    if (!config.check<std::string>("device"))
    {
        // TODO(traversaro): avoid to call std::cerr here, and 
        // implement a way for users to configure the logging system
        std::cerr << "dinrail::Device: missing required parameter 'device'" << std::endl;
        return false;
    }

    const std::string deviceName = config.find("device").as<std::string>();


    const std::string libraryName = getSharedlibppLibraryNameFromDeviceName(deviceName);
    const std::string factoryName = getSharedlibppFactoryNameFromDeviceName(deviceName);
    const auto pluginSearchPaths = getPluginSearchPaths();

    m_pimpl->deviceFactory = std::make_unique<
        sharedlibpp::SharedLibraryClassFactory<dinrail::IDevice>>(SHLIBPP_DEFAULT_START_CHECK,
                                                                   SHLIBPP_DEFAULT_END_CHECK,
                                                                   SHLIBPP_DEFAULT_SYSTEM_VERSION,
                                                                   factoryName.c_str());

    for (const auto& path : pluginSearchPaths)
    {
        m_pimpl->deviceFactory->extendSearchPath(path.string());
    }

    bool ok = m_pimpl->deviceFactory->open(libraryName.c_str(), factoryName.c_str());
    ok = ok && m_pimpl->deviceFactory->isValid();

    if (!ok)
    {
        std::cerr << "dinrail::Device: impossible to find library for " 
                  << deviceName << " device. Searched library name: " << libraryName 
                  << ", factory name: " << factoryName
                  << std::endl;
        return false;
    }

    auto device = make_factory_unique(*(m_pimpl->deviceFactory));

    if (!device)
    {
        std::cerr << "dinrail::Device: impossible to create factory for " 
                  << deviceName << " device. Searched library name: " << libraryName 
                  << ", factory name: " << factoryName
                  << std::endl;
        return false;
    }

    ok = device->open(config);

    if (!ok)
    {
        std::cerr << "dinrail::Device: open return false for " 
                  << deviceName << " device. Searched library name: " << libraryName 
                  << ", factory name: " << factoryName
                  << std::endl;
        return false;
    }

    m_pimpl->device = std::move(device);
    m_pimpl->isValid = true;
    return true;
}

bool Device::close()
{
    bool result = true;
    if (m_pimpl->device)
    {
        result = m_pimpl->device->close();
    }

    m_pimpl->isValid = false;

    // This ensures that the device is deleted first (as it uses the factory's destroy method),
    // before unloading the library in the factory destructor.
    m_pimpl->device.reset();
    m_pimpl->deviceFactory.reset();
    return result;
}

bool Device::isValid() const
{
    return m_pimpl->isValid;
}

IDevice* Device::getImplementation()
{
    return m_pimpl->device.get();
}

} // namespace dinrail
