// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>

namespace dinrail
{

Device::Device() = default;

Device::~Device() = default;

bool Device::open(const Property& /*config*/)
{
    m_isValid = true;
    return true;
}

bool Device::close()
{
    m_isValid = false;
    return true;
}

bool Device::isValid() const
{
    return m_isValid;
}

Device* Device::getImplementation()
{
    return this;
}

} // namespace dinrail
