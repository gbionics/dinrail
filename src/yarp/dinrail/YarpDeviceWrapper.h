// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPDEVICEWRAPPER_H
#define DINRAIL_YARPDEVICEWRAPPER_H

#include <dinrail/IDevice.h>
#include <dinrail/IInterfaceView.h>

#include <yarp/dev/PolyDriver.h>

#include <memory>
#include <typeinfo>

namespace dinrail
{

/**
 * @brief Wrap a yarp::dev::PolyDriver as a dinrail::IDevice.
 *
 * Native YARP interfaces implemented by the wrapped device are exposed through
 * IInterfaceView so that dinrail::Device::view() can resolve them.
 */
class YarpDeviceWrapper final : public IDevice, public IInterfaceView
{
public:
    explicit YarpDeviceWrapper(std::unique_ptr<yarp::dev::PolyDriver> yarpDevice);
    ~YarpDeviceWrapper() override;

    bool open(const Parameters& config) override;
    bool close() override;

    void* viewInterface(const std::type_info& interfaceType) override;

private:
    std::unique_ptr<yarp::dev::PolyDriver> m_yarpDevice;
};

} // namespace dinrail

#endif // DINRAIL_YARPDEVICEWRAPPER_H
