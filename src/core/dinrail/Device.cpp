// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IInterfaceQueryable.h>
#include <dinrail/RuntimeContext.h>
#include <dinrail/IDevice.h>

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
    std::unique_ptr<dinrail::IDevice> driver;
};

Device::Device()
    : Device(RuntimeContext::get_default())
{
}

Device::Device(const RuntimeContext& context)
    : m_pimpl(std::make_unique<Impl>(context))
{
}

Device::~Device() = default;

bool Device::open(const Parameters& config)
{
    m_pimpl->driver.reset();
    m_pimpl->isValid = false;

    m_pimpl->driver = m_pimpl->context.createDevice(config);
    m_pimpl->isValid = (m_pimpl->driver != nullptr);
    return m_pimpl->isValid;
}

bool Device::close()
{
    bool result = true;
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
    return m_pimpl->isValid;
}

IDevice* Device::getDriver()
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

void* Device::queryAdapter(const std::type_info& interfaceType)
{
    void* ptr = m_pimpl->context.queryAdapter(m_pimpl->driver.get(), interfaceType);
    if (ptr != nullptr)
    {
        return ptr;
    }

    if (!m_pimpl->driver)
    {
        return nullptr;
    }

    auto* queryable = dynamic_cast<IInterfaceQueryable*>(m_pimpl->driver.get());
    if (queryable == nullptr)
    {
        return nullptr;
    }

    return queryable->queryInterface(interfaceType);
}

} // namespace dinrail
