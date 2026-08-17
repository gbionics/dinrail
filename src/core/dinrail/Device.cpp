// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IDevice.h>
#include <dinrail/IInterfaceTranslation.h>
#include <dinrail/RuntimeContext.h>

#include <memory>
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
    std::unordered_map<std::type_index, std::unique_ptr<IInterfaceTranslation>> translations;
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

    m_pimpl->translations.clear();
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
        m_pimpl->translations.clear();
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

void* Device::viewTranslatedInterface(const std::type_info& interfaceType)
{
    if (!m_pimpl || !m_pimpl->driver)
    {
        return nullptr;
    }

    const std::type_index key(interfaceType);
    const auto existing = m_pimpl->translations.find(key);
    if (existing != m_pimpl->translations.end())
    {
        return existing->second->getInterface();
    }

    auto translation = m_pimpl->context.createInterfaceTranslation(*m_pimpl->driver, interfaceType);
    if (!translation)
    {
        return nullptr;
    }

    void* translatedInterface = translation->getInterface();
    if (translatedInterface == nullptr)
    {
        return nullptr;
    }

    m_pimpl->translations.emplace(key, std::move(translation));
    return translatedInterface;
}

} // namespace dinrail
