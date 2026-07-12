// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPDEVICEWRAPPER_H
#define DINRAIL_YARPDEVICEWRAPPER_H

#include <dinrail/IDevice.h>
#include <dinrail/IInterfaceQueryable.h>
#include <dinrail/YarpNativeInterfaceRegistry.h>

#include <yarp/dev/PolyDriver.h>

#include <memory>
#include <typeinfo>

namespace dinrail
{

class YarpAxisInfoAdapter;

/**
 * @brief Wrap a yarp::dev::PolyDriver as a dinrail::IDevice and expose selected dinrail interfaces.
 */
class YarpDeviceWrapper final : public IDevice, public IInterfaceQueryable
{
public:
    explicit YarpDeviceWrapper(
        std::unique_ptr<yarp::dev::PolyDriver> yarpDevice,
        std::shared_ptr<const YarpNativeInterfaceRegistry> nativeInterfaceRegistry);
    ~YarpDeviceWrapper() override;

    bool open(const Parameters& config) override;
    bool close() override;
    void* queryInterface(const std::type_info& interfaceType) override;

    void* getAdapter(const std::type_info& interfaceType);

private:
    std::unique_ptr<yarp::dev::PolyDriver> m_yarpDevice;
    std::unique_ptr<YarpAxisInfoAdapter> m_axisInfoAdapter;
    std::shared_ptr<const YarpNativeInterfaceRegistry> m_nativeInterfaceRegistry;
};

} // namespace dinrail

#endif // DINRAIL_YARPDEVICEWRAPPER_H