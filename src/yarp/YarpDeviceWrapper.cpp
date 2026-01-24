// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "YarpDeviceWrapper.h"
#include "YarpPositionDirectAdapter.h"
#include "YarpEncodersAdapter.h"
#include "YarpControlModeAdapter.h"

#include <dinrail/IPositionDirect.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IControlMode.h>

#include <typeinfo>

namespace dinrail
{

YarpDeviceWrapper::YarpDeviceWrapper(std::unique_ptr<yarp::dev::PolyDriver> yarpDevice)
    : m_yarpDevice(std::move(yarpDevice))
{
    // Create adapters for each interface
    if (m_yarpDevice && m_yarpDevice->isValid())
    {
        m_positionDirectAdapter = std::make_unique<YarpPositionDirectAdapter>(m_yarpDevice.get());
        m_encodersAdapter = std::make_unique<YarpEncodersAdapter>(m_yarpDevice.get());
        m_controlModeAdapter = std::make_unique<YarpControlModeAdapter>(m_yarpDevice.get());
    }
}

YarpDeviceWrapper::~YarpDeviceWrapper() = default;

bool YarpDeviceWrapper::open(const Property& config)
{
    // The YARP device is already opened in the compatibility layer
    // This method is called after the device is created
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
    // This should not be called for wrapper instances
    return nullptr;
}

void* YarpDeviceWrapper::getAdapter(const std::type_info& interfaceType)
{
    if (interfaceType == typeid(IPositionDirect)) {
        return m_positionDirectAdapter.get();
    }
    if (interfaceType == typeid(IEncoders)) {
        return m_encodersAdapter.get();
    }
    if (interfaceType == typeid(IControlMode)) {
        return m_controlModeAdapter.get();
    }
    return nullptr;
}

} // namespace dinrail
