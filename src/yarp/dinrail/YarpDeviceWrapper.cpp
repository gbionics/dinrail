// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpDeviceWrapper.h>

#include <dinrail/IAxisInfo.h>
#include <dinrail/YarpAxisInfoAdapter.h>

namespace dinrail
{

YarpDeviceWrapper::YarpDeviceWrapper(std::unique_ptr<yarp::dev::PolyDriver> yarpDevice)
    : m_yarpDevice(std::move(yarpDevice))
{
    if (m_yarpDevice && m_yarpDevice->isValid())
    {
        m_axisInfoAdapter = std::make_unique<YarpAxisInfoAdapter>(m_yarpDevice.get());
    }
}

YarpDeviceWrapper::~YarpDeviceWrapper() = default;

bool YarpDeviceWrapper::open(const Parameters& config)
{
    static_cast<void>(config);
    return m_yarpDevice && m_yarpDevice->isValid();
}

bool YarpDeviceWrapper::close()
{
    if (m_yarpDevice)
    {
        return m_yarpDevice->close();
    }
    return false;
}

IDevice* YarpDeviceWrapper::allocateInstance() const
{
    return nullptr;
}

void* YarpDeviceWrapper::getAdapter(const std::type_info& interfaceType)
{
    if (interfaceType == typeid(IAxisInfo))
    {
        return m_axisInfoAdapter.get();
    }
    return nullptr;
}

} // namespace dinrail
