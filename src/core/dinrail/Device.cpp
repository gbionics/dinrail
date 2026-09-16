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
    std::mutex adaptersMutex;
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

    m_pimpl->adapters.clear();
    m_pimpl->driver.reset();
    m_pimpl->isValid = false;

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

    bool result = true;
    if (m_pimpl->driver)
    {
        m_pimpl->adapters.clear();
        result = m_pimpl->driver->close();
    }

    m_pimpl->isValid = false;
    m_pimpl->driver.reset();
    return result;
}

bool Device::isValid() const
{
    return m_pimpl && m_pimpl->isValid;
}

IDevice* Device::getImplementation()
{
    return m_pimpl ? m_pimpl->driver.get() : nullptr;
}

void* Device::viewAdaptedInterface(const std::type_info& interfaceType)
{
    if (!m_pimpl || !m_pimpl->driver)
    {
        return nullptr;
    }

    const std::type_index key(interfaceType);
    {
        std::lock_guard<std::mutex> lock(m_pimpl->adaptersMutex);
        const auto existing = m_pimpl->adapters.find(key);
        if (existing != m_pimpl->adapters.end())
        {
            return existing->second->getInterface();
        }
    }

    // Plugin callbacks can query devices too. Never hold the cache mutex while
    // entering the runtime or constructing an adapter.
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

    {
        std::lock_guard<std::mutex> lock(m_pimpl->adaptersMutex);
        const auto [entry, inserted] = m_pimpl->adapters.try_emplace(key, std::move(adapter));
        if (!inserted)
        {
            adaptedInterface = entry->second->getInterface();
        }
    }
    // If another query won the race, destroy the unused adapter outside the lock.
    return adaptedInterface;
}

} // namespace dinrail
