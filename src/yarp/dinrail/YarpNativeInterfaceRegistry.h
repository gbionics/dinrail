// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPNATIVEINTERFACEREGISTRY_H
#define DINRAIL_YARPNATIVEINTERFACEREGISTRY_H

#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/DeviceDriver.h>

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace dinrail
{

class YarpNativeInterfaceRegistry
{
public:
    using Driver = yarp::dev::DeviceDriver;
    using Probe = void* (*)(Driver*) noexcept;

    template <class Interface> void add()
    {
        m_probes.insert_or_assign(std::type_index(typeid(Interface)),
                                  +[](Driver* driver) noexcept -> void* {
                                      if (driver == nullptr)
                                      {
                                          return nullptr;
                                      }

                                      return dynamic_cast<Interface*>(driver);
                                  });
    }

    void* query(Driver* driver, const std::type_info& interfaceType) const noexcept
    {
        if (driver == nullptr)
        {
            return nullptr;
        }

        const auto it = m_probes.find(std::type_index(interfaceType));
        if (it == m_probes.end())
        {
            return nullptr;
        }

        return it->second(driver);
    }

private:
    std::unordered_map<std::type_index, Probe> m_probes;
};

inline std::shared_ptr<const YarpNativeInterfaceRegistry> createDefaultYarpNativeInterfaceRegistry()
{
    auto registry = std::make_shared<YarpNativeInterfaceRegistry>();

    registry->add<yarp::dev::DeviceDriver>();
    registry->add<yarp::dev::IAxisInfo>();
    registry->add<yarp::dev::IEncoders>();
    registry->add<yarp::dev::IEncodersTimed>();
    registry->add<yarp::dev::ITorqueControl>();
    registry->add<yarp::dev::IImpedanceControl>();
    registry->add<yarp::dev::IVelocityControl>();
    registry->add<yarp::dev::IPositionControl>();
    registry->add<yarp::dev::ICurrentControl>();
    registry->add<yarp::dev::IPWMControl>();
    registry->add<yarp::dev::IPidControl>();
    registry->add<yarp::dev::IPositionDirect>();
    registry->add<yarp::dev::IInteractionMode>();
    registry->add<yarp::dev::IMotorEncoders>();
    registry->add<yarp::dev::IMotor>();
    registry->add<yarp::dev::IRemoteVariables>();
    registry->add<yarp::dev::IControlDebug>();
    registry->add<yarp::dev::IControlCalibration>();
    registry->add<yarp::dev::IAmplifierControl>();
    registry->add<yarp::dev::IControlLimits>();
    registry->add<yarp::dev::IControlMode>();
    registry->add<yarp::dev::IJointCoupling>();
    registry->add<yarp::dev::IJointFault>();

    return registry;
}

} // namespace dinrail

#endif // DINRAIL_YARPNATIVEINTERFACEREGISTRY_H