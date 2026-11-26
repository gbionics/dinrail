// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DRDEVICEYARP_H
#define DINRAIL_DRDEVICEYARP_H

#include <dinrail/Device.h>
#include <dinrail/IWrappedDinrailDevice.h>
#include <dinrail/MultipleAnalogSensorsInterfaces.h>
#include <dinrail/YarpMultipleAnalogSensorsAdapters.h>

#include <yarp/dev/DeviceDriver.h>

/**
 * @brief YARP device that opens any dinrail device, so that YARP network wrappers can be attached
 * to it.
 *
 * The dinrail-aware wrappers (for example dr_rgbd_nws_yarp) use the native interfaces of the
 * wrapped device through dinrail::IWrappedDinrailDevice. The inertial sensors of the wrapped
 * device are also exposed through the YARP multiple analog sensors interfaces, so that they can
 * be published by multipleanalogsensorsserver. The inertial sensors that the wrapped device does
 * not implement are exposed as interfaces without sensors.
 */
class DrDeviceYarp final
    : public yarp::dev::DeviceDriver,
      public dinrail::IWrappedDinrailDevice,
      public dinrail::InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes,
                                       dinrail::IThreeAxisGyroscopes>,
      public dinrail::InterfaceAdapter<yarp::dev::IThreeAxisLinearAccelerometers,
                                       dinrail::IThreeAxisLinearAccelerometers>,
      public dinrail::InterfaceAdapter<yarp::dev::IThreeAxisAngularAccelerometers,
                                       dinrail::IThreeAxisAngularAccelerometers>,
      public dinrail::InterfaceAdapter<yarp::dev::IThreeAxisMagnetometers,
                                       dinrail::IThreeAxisMagnetometers>,
      public dinrail::InterfaceAdapter<yarp::dev::IOrientationSensors, dinrail::IOrientationSensors>
{
public:
    DrDeviceYarp();
    DrDeviceYarp(const DrDeviceYarp&) = delete;
    DrDeviceYarp& operator=(const DrDeviceYarp&) = delete;
    ~DrDeviceYarp() override;

    bool open(yarp::os::Searchable& config) override;
    bool close() override;

    dinrail::Device& wrappedDinrailDevice() override;

private:
    /// Inertial sensors interfaces without sensors, used for the ones the device does not have.
    class NoInertialSensors final : public dinrail::IThreeAxisGyroscopes,
                                    public dinrail::IThreeAxisLinearAccelerometers,
                                    public dinrail::IThreeAxisAngularAccelerometers,
                                    public dinrail::IThreeAxisMagnetometers,
                                    public dinrail::IOrientationSensors
    {
    public:
        std::size_t getNrOfThreeAxisGyroscopes() const override;
        dinrail::MAS_status getThreeAxisGyroscopeStatus(std::size_t index) const override;
        bool getThreeAxisGyroscopeName(std::size_t index, std::string& name) const override;
        bool getThreeAxisGyroscopeFrameName(std::size_t index, std::string& frame) const override;
        bool getThreeAxisGyroscopeMeasure(std::size_t index,
                                          dinrail::VectorProxy<double>::Ref out,
                                          double& timestamp) const override;

        std::size_t getNrOfThreeAxisLinearAccelerometers() const override;
        dinrail::MAS_status getThreeAxisLinearAccelerometerStatus(std::size_t index) const override;
        bool
        getThreeAxisLinearAccelerometerName(std::size_t index, std::string& name) const override;
        bool getThreeAxisLinearAccelerometerFrameName(std::size_t index,
                                                      std::string& frame) const override;
        bool getThreeAxisLinearAccelerometerMeasure(std::size_t index,
                                                    dinrail::VectorProxy<double>::Ref out,
                                                    double& timestamp) const override;

        std::size_t getNrOfThreeAxisAngularAccelerometers() const override;
        dinrail::MAS_status
        getThreeAxisAngularAccelerometerStatus(std::size_t index) const override;
        bool
        getThreeAxisAngularAccelerometerName(std::size_t index, std::string& name) const override;
        bool getThreeAxisAngularAccelerometerFrameName(std::size_t index,
                                                       std::string& frame) const override;
        bool getThreeAxisAngularAccelerometerMeasure(std::size_t index,
                                                     dinrail::VectorProxy<double>::Ref out,
                                                     double& timestamp) const override;

        std::size_t getNrOfThreeAxisMagnetometers() const override;
        dinrail::MAS_status getThreeAxisMagnetometerStatus(std::size_t index) const override;
        bool getThreeAxisMagnetometerName(std::size_t index, std::string& name) const override;
        bool
        getThreeAxisMagnetometerFrameName(std::size_t index, std::string& frame) const override;
        bool getThreeAxisMagnetometerMeasure(std::size_t index,
                                             dinrail::VectorProxy<double>::Ref out,
                                             double& timestamp) const override;

        std::size_t getNrOfOrientationSensors() const override;
        dinrail::MAS_status getOrientationSensorStatus(std::size_t index) const override;
        bool getOrientationSensorName(std::size_t index, std::string& name) const override;
        bool getOrientationSensorFrameName(std::size_t index, std::string& frame) const override;
        bool getOrientationSensorMeasureAsRollPitchYaw(std::size_t index,
                                                       dinrail::VectorProxy<double>::Ref out,
                                                       double& timestamp) const override;
    };

    // Binds the inertial adapters to the wrapped device, or to m_noInertialSensors.
    void bindInertialAdapters();
    template <class Adapter> void bindInertialAdapter();

    dinrail::Device m_device;
    NoInertialSensors m_noInertialSensors;
};

#endif // DINRAIL_DRDEVICEYARP_H
