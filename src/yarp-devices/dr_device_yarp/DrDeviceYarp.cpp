// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "DrDeviceYarp.h"

#include <dinrail/SpdlogHelpers.h>
#include <dinrail/YarpPropertyConverter.h>

#include <yarp/os/Property.h>

namespace
{

spdlog::logger& logger()
{
    static const auto instance = [] {
        auto created = dinrail::createOrGetLogger("dinrail.device_yarp");
        return created ? created : spdlog::default_logger();
    }();
    return *instance;
}

} // namespace

DrDeviceYarp::DrDeviceYarp()
{
    bindInertialAdapters();
}

DrDeviceYarp::~DrDeviceYarp()
{
    close();
}

bool DrDeviceYarp::open(yarp::os::Searchable& config)
{
    close();
    if (!config.check("subdevice") || !config.find("subdevice").isString())
    {
        logger().error("The parameter 'subdevice' (name of the dinrail device) is mandatory.");
        return false;
    }

    yarp::os::Property subdeviceConfig;
    subdeviceConfig.fromString(config.toString());
    subdeviceConfig.unput("device");
    subdeviceConfig.unput("subdevice");
    // Added by yarprobotinterface and dinrail-runner to every device.
    subdeviceConfig.unput("id");
    subdeviceConfig.unput("robotName");
    dinrail::Parameters parameters
        = dinrail::YarpPropertyConverter::toDinrailParameters(subdeviceConfig);
    parameters.put("device", config.find("subdevice").asString());
    parameters.put("dinrail_device_type", "dinrail");
    if (!m_device.open(parameters))
    {
        logger().error("Unable to open the dinrail device '{}'.",
                       config.find("subdevice").asString());
        return false;
    }

    bindInertialAdapters();
    return true;
}

bool DrDeviceYarp::close()
{
    const bool ok = !m_device.isValid() || m_device.close();
    bindInertialAdapters();
    return ok;
}

dinrail::Device& DrDeviceYarp::wrappedDinrailDevice()
{
    return m_device;
}

template <class Adapter> void DrDeviceYarp::bindInertialAdapter()
{
    typename Adapter::SourceInterface* source = nullptr;
    if (m_device.isValid() && m_device.view(source) && source != nullptr)
    {
        static_cast<Adapter&>(*this).bind(*source);
    } else
    {
        static_cast<Adapter&>(*this).bind(m_noInertialSensors);
    }
}

void DrDeviceYarp::bindInertialAdapters()
{
    bindInertialAdapter<
        dinrail::InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes, dinrail::IThreeAxisGyroscopes>>();
    bindInertialAdapter<dinrail::InterfaceAdapter<yarp::dev::IThreeAxisLinearAccelerometers,
                                                  dinrail::IThreeAxisLinearAccelerometers>>();
    bindInertialAdapter<dinrail::InterfaceAdapter<yarp::dev::IThreeAxisAngularAccelerometers,
                                                  dinrail::IThreeAxisAngularAccelerometers>>();
    bindInertialAdapter<dinrail::InterfaceAdapter<yarp::dev::IThreeAxisMagnetometers,
                                                  dinrail::IThreeAxisMagnetometers>>();
    bindInertialAdapter<
        dinrail::InterfaceAdapter<yarp::dev::IOrientationSensors, dinrail::IOrientationSensors>>();
}

std::size_t DrDeviceYarp::NoInertialSensors::getNrOfThreeAxisGyroscopes() const
{
    return 0;
}

dinrail::MAS_status
DrDeviceYarp::NoInertialSensors::getThreeAxisGyroscopeStatus(std::size_t /*index*/) const
{
    return dinrail::MAS_UNKNOWN;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisGyroscopeName(std::size_t /*index*/,
                                                                std::string& /*name*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisGyroscopeFrameName(std::size_t /*index*/,
                                                                     std::string& /*frame*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisGyroscopeMeasure(
    std::size_t /*index*/, dinrail::VectorProxy<double>::Ref /*out*/, double& /*timestamp*/) const
{
    return false;
}

std::size_t DrDeviceYarp::NoInertialSensors::getNrOfThreeAxisLinearAccelerometers() const
{
    return 0;
}

dinrail::MAS_status
DrDeviceYarp::NoInertialSensors::getThreeAxisLinearAccelerometerStatus(std::size_t /*index*/) const
{
    return dinrail::MAS_UNKNOWN;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisLinearAccelerometerName(
    std::size_t /*index*/, std::string& /*name*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisLinearAccelerometerFrameName(
    std::size_t /*index*/, std::string& /*frame*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisLinearAccelerometerMeasure(
    std::size_t /*index*/, dinrail::VectorProxy<double>::Ref /*out*/, double& /*timestamp*/) const
{
    return false;
}

std::size_t DrDeviceYarp::NoInertialSensors::getNrOfThreeAxisAngularAccelerometers() const
{
    return 0;
}

dinrail::MAS_status
DrDeviceYarp::NoInertialSensors::getThreeAxisAngularAccelerometerStatus(std::size_t /*index*/) const
{
    return dinrail::MAS_UNKNOWN;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisAngularAccelerometerName(
    std::size_t /*index*/, std::string& /*name*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisAngularAccelerometerFrameName(
    std::size_t /*index*/, std::string& /*frame*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisAngularAccelerometerMeasure(
    std::size_t /*index*/, dinrail::VectorProxy<double>::Ref /*out*/, double& /*timestamp*/) const
{
    return false;
}

std::size_t DrDeviceYarp::NoInertialSensors::getNrOfThreeAxisMagnetometers() const
{
    return 0;
}

dinrail::MAS_status
DrDeviceYarp::NoInertialSensors::getThreeAxisMagnetometerStatus(std::size_t /*index*/) const
{
    return dinrail::MAS_UNKNOWN;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisMagnetometerName(std::size_t /*index*/,
                                                                   std::string& /*name*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisMagnetometerFrameName(
    std::size_t /*index*/, std::string& /*frame*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getThreeAxisMagnetometerMeasure(
    std::size_t /*index*/, dinrail::VectorProxy<double>::Ref /*out*/, double& /*timestamp*/) const
{
    return false;
}

std::size_t DrDeviceYarp::NoInertialSensors::getNrOfOrientationSensors() const
{
    return 0;
}

dinrail::MAS_status
DrDeviceYarp::NoInertialSensors::getOrientationSensorStatus(std::size_t /*index*/) const
{
    return dinrail::MAS_UNKNOWN;
}

bool DrDeviceYarp::NoInertialSensors::getOrientationSensorName(std::size_t /*index*/,
                                                               std::string& /*name*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getOrientationSensorFrameName(std::size_t /*index*/,
                                                                    std::string& /*frame*/) const
{
    return false;
}

bool DrDeviceYarp::NoInertialSensors::getOrientationSensorMeasureAsRollPitchYaw(
    std::size_t /*index*/, dinrail::VectorProxy<double>::Ref /*out*/, double& /*timestamp*/) const
{
    return false;
}
