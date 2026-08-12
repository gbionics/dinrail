// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpDeviceWrapper.h>

#include <dinrail/RuntimeDynamicCast.h>

namespace dinrail
{

YarpDeviceWrapper::YarpDeviceWrapper(std::unique_ptr<yarp::dev::PolyDriver> yarpDevice)
    : m_yarpDevice(std::move(yarpDevice))
{
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

void* YarpDeviceWrapper::viewInterface(const std::type_info& interfaceType)
{
    if (m_yarpDevice == nullptr)
    {
        return nullptr;
    }

    // Any native YARP interface implemented by the wrapped device is resolved in
    // a single runtime cast, without enumerating the interface types.
    return runtimeDynamicCast(makePolymorphicView(m_yarpDevice->getImplementation()),
                              interfaceType);
}

} // namespace dinrail
