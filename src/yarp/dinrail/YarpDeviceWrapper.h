// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPDEVICEWRAPPER_H
#define DINRAIL_YARPDEVICEWRAPPER_H

#include <dinrail/IDevice.h>

#include <yarp/dev/PolyDriver.h>

#include <memory>
#include <typeinfo>

namespace dinrail
{

class YarpAxisInfoAdapter;

class YarpDeviceWrapper : public IDevice
{
public:
    explicit YarpDeviceWrapper(std::unique_ptr<yarp::dev::PolyDriver> yarpDevice);
    ~YarpDeviceWrapper() override;

    bool open(const Parameters& config) override;
    bool close() override;
    IDevice* allocateInstance() const override;

    void* getAdapter(const std::type_info& interfaceType);

private:
    std::unique_ptr<yarp::dev::PolyDriver> m_yarpDevice;
    std::unique_ptr<YarpAxisInfoAdapter> m_axisInfoAdapter;
};

} // namespace dinrail

#endif // DINRAIL_YARPDEVICEWRAPPER_H
