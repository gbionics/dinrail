// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IDevice.h>
#include <dinrail/IInterfaceAdapter.h>
#include <dinrail/RuntimeContext.h>

#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace dinrail
{

struct Device::Impl
{
    explicit Impl(RuntimeContext context)
        : context(std::move(context))
    {
    }

    RuntimeContext context;
    bool isValid{false};
    FactoryUniquePtr<dinrail::IDevice> driver;
    std::mutex mutex;
    std::unordered_map<std::type_index, std::unique_ptr<IInterfaceAdapter>> adapters;
};

Device::Device()
    : Device(RuntimeContext::getDefault())
{
}

Device::Device(const RuntimeContext& context)
    : m_pimpl(std::make_unique<Impl>(context))
{
}

Device::~Device()
{
    close();
}

bool Device::open(const Parameters& config)
{
    if (!m_pimpl)
    {
        return false;
    }

    std::unique_lock<std::mutex> lock(m_pimpl->mutex);
    closeWithoutLock();

    m_pimpl->driver = m_pimpl->context.createDevice(config);
    m_pimpl->isValid = (m_pimpl->driver != nullptr);
    return m_pimpl->isValid;
}

bool Device::close()
{
    if (!m_pimpl)
    {
        return true;
    }

    std::unique_lock<std::mutex> lock(m_pimpl->mutex);
    return closeWithoutLock();
}

std::unique_lock<std::mutex> Device::lockImpl() const
{
    return std::unique_lock<std::mutex>(m_pimpl->mutex);
}

bool Device::closeWithoutLock()
{
    bool result = true;
    m_pimpl->adapters.clear();
    if (m_pimpl->driver)
    {
        result = m_pimpl->driver->close();
    }

    m_pimpl->isValid = false;
    m_pimpl->driver.reset();
    return result;
}

bool Device::isValid() const
{
    if (!m_pimpl)
    {
        return false;
    }
    std::unique_lock<std::mutex> lock(m_pimpl->mutex);
    return m_pimpl->isValid;
}

IDevice* Device::getImplementationWithoutLock()
{
    return m_pimpl->driver.get();
}

void* Device::viewAdaptedInterfaceWithoutLock(const std::type_info& interfaceType)
{
    if (!m_pimpl->driver)
    {
        return nullptr;
    }

    const std::type_index key(interfaceType);
    const auto existing = m_pimpl->adapters.find(key);
    if (existing != m_pimpl->adapters.end())
    {
        return existing->second->getInterface();
    }

    auto adapter = m_pimpl->context.createInterfaceAdapter(*m_pimpl->driver, interfaceType);
    if (!adapter)
    {
        return nullptr;
    }

    void* adaptedInterface = adapter->getInterface();
    if (adaptedInterface == nullptr)
    {
        return nullptr;
    }

    m_pimpl->adapters.emplace(key, std::move(adapter));
    return adaptedInterface;
}

} // namespace dinrail
