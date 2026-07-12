// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpDeviceWrapper.h>

#include <dinrail/IAxisInfo.h>
#include <dinrail/YarpAxisInfoAdapter.h>

namespace dinrail
{

YarpDeviceWrapper::YarpDeviceWrapper(
    std::unique_ptr<yarp::dev::PolyDriver> yarpDevice,
    std::shared_ptr<const YarpNativeInterfaceRegistry> nativeInterfaceRegistry)
    : m_yarpDevice(std::move(yarpDevice))
    , m_nativeInterfaceRegistry(std::move(nativeInterfaceRegistry))
{
    if (m_yarpDevice != nullptr && m_yarpDevice->isValid())
    {
        m_axisInfoAdapter = std::make_unique<YarpAxisInfoAdapter>(m_yarpDevice.get());
    }
}

YarpDeviceWrapper::~YarpDeviceWrapper() = default;

bool YarpDeviceWrapper::open(const Parameters& config)
{
    static_cast<void>(config);
    return m_yarpDevice != nullptr && m_yarpDevice->isValid();
}

bool YarpDeviceWrapper::close()
{
    if (m_yarpDevice == nullptr)
    {
        return false;
    }

    return m_yarpDevice->close();
}

void* YarpDeviceWrapper::getAdapter(const std::type_info& interfaceType)
{
    if (interfaceType == typeid(IAxisInfo))
    {
        return m_axisInfoAdapter.get();
    }

    return nullptr;
}

void* YarpDeviceWrapper::queryInterface(const std::type_info& interfaceType)
{
    if (!m_yarpDevice)
    {
        return nullptr;
    }

    if (!m_nativeInterfaceRegistry)
    {
        return nullptr;
    }

    yarp::dev::DeviceDriver* implementation = m_yarpDevice->getImplementation();
    return m_nativeInterfaceRegistry->query(implementation, interfaceType);
}

} // namespace dinrail