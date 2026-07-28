// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IDevice.h>
#include <dinrail/RuntimeContext.h>

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

IDevice* Device::getImplementation()
{
    return m_pimpl->driver.get();
}

} // namespace dinrail
