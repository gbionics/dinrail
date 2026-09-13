// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DINRAILMULTIPLEANALOGSENSORSINTERFACEADAPTERS_H
#define DINRAIL_DINRAILMULTIPLEANALOGSENSORSINTERFACEADAPTERS_H

#include <dinrail/InterfaceAdapter.h>
#include <dinrail/MultipleAnalogSensorsInterfaces.h>
#include <yarp/dev/MultipleAnalogSensorsInterfaces.h>
#include <yarp/sig/Vector.h>

namespace dinrail
{
template <>
class InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes, dinrail::IThreeAxisGyroscopes>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IThreeAxisGyroscopes,
                                           dinrail::IThreeAxisGyroscopes>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfThreeAxisGyroscopes() const override;
    yarp::dev::MAS_status getThreeAxisGyroscopeStatus(std::size_t index) const override;
    bool getThreeAxisGyroscopeName(std::size_t index, std::string& name) const override;
    bool getThreeAxisGyroscopeFrameName(std::size_t index, std::string& frame) const override;
    bool getThreeAxisGyroscopeMeasure(std::size_t index,
                                      yarp::sig::Vector& output,
                                      double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::IThreeAxisLinearAccelerometers,
                       dinrail::IThreeAxisLinearAccelerometers>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IThreeAxisLinearAccelerometers,
                                           dinrail::IThreeAxisLinearAccelerometers>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfThreeAxisLinearAccelerometers() const override;
    yarp::dev::MAS_status getThreeAxisLinearAccelerometerStatus(std::size_t index) const override;
    bool getThreeAxisLinearAccelerometerName(std::size_t index, std::string& name) const override;
    bool
    getThreeAxisLinearAccelerometerFrameName(std::size_t index, std::string& frame) const override;
    bool getThreeAxisLinearAccelerometerMeasure(std::size_t index,
                                                yarp::sig::Vector& output,
                                                double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::IThreeAxisAngularAccelerometers,
                       dinrail::IThreeAxisAngularAccelerometers>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IThreeAxisAngularAccelerometers,
                                           dinrail::IThreeAxisAngularAccelerometers>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfThreeAxisAngularAccelerometers() const override;
    yarp::dev::MAS_status getThreeAxisAngularAccelerometerStatus(std::size_t index) const override;
    bool getThreeAxisAngularAccelerometerName(std::size_t index, std::string& name) const override;
    bool
    getThreeAxisAngularAccelerometerFrameName(std::size_t index, std::string& frame) const override;
    bool getThreeAxisAngularAccelerometerMeasure(std::size_t index,
                                                 yarp::sig::Vector& output,
                                                 double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::IThreeAxisMagnetometers, dinrail::IThreeAxisMagnetometers>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IThreeAxisMagnetometers,
                                           dinrail::IThreeAxisMagnetometers>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfThreeAxisMagnetometers() const override;
    yarp::dev::MAS_status getThreeAxisMagnetometerStatus(std::size_t index) const override;
    bool getThreeAxisMagnetometerName(std::size_t index, std::string& name) const override;
    bool getThreeAxisMagnetometerFrameName(std::size_t index, std::string& frame) const override;
    bool getThreeAxisMagnetometerMeasure(std::size_t index,
                                         yarp::sig::Vector& output,
                                         double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::IPositionSensors, dinrail::IPositionSensors>
    : public InterfaceAdapterBase<yarp::dev::IPositionSensors, dinrail::IPositionSensors>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    std::size_t getNrOfPositionSensors() const override;
    yarp::dev::MAS_status getPositionSensorStatus(std::size_t index) const override;
    bool getPositionSensorName(std::size_t index, std::string& name) const override;
    bool getPositionSensorFrameName(std::size_t index, std::string& frame) const override;
    bool getPositionSensorMeasure(std::size_t index,
                                  yarp::sig::Vector& output,
                                  double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::ILinearVelocitySensors, dinrail::ILinearVelocitySensors>
    : public dinrail::InterfaceAdapterBase<yarp::dev::ILinearVelocitySensors,
                                           dinrail::ILinearVelocitySensors>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfLinearVelocitySensors() const override;
    yarp::dev::MAS_status getLinearVelocitySensorStatus(std::size_t index) const override;
    bool getLinearVelocitySensorName(std::size_t index, std::string& name) const override;
    bool getLinearVelocitySensorFrameName(std::size_t index, std::string& frame) const override;
    bool getLinearVelocitySensorMeasure(std::size_t index,
                                        yarp::sig::Vector& output,
                                        double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::IOrientationSensors, dinrail::IOrientationSensors>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IOrientationSensors,
                                           dinrail::IOrientationSensors>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfOrientationSensors() const override;
    yarp::dev::MAS_status getOrientationSensorStatus(std::size_t index) const override;
    bool getOrientationSensorName(std::size_t index, std::string& name) const override;
    bool getOrientationSensorFrameName(std::size_t index, std::string& frame) const override;
    bool getOrientationSensorMeasureAsRollPitchYaw(std::size_t index,
                                                   yarp::sig::Vector& output,
                                                   double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::ISixAxisForceTorqueSensors, dinrail::ISixAxisForceTorqueSensors>
    : public dinrail::InterfaceAdapterBase<yarp::dev::ISixAxisForceTorqueSensors,
                                           dinrail::ISixAxisForceTorqueSensors>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfSixAxisForceTorqueSensors() const override;
    yarp::dev::MAS_status getSixAxisForceTorqueSensorStatus(std::size_t index) const override;
    bool getSixAxisForceTorqueSensorName(std::size_t index, std::string& name) const override;
    bool getSixAxisForceTorqueSensorFrameName(std::size_t index, std::string& frame) const override;
    bool getSixAxisForceTorqueSensorMeasure(std::size_t index,
                                            yarp::sig::Vector& output,
                                            double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::IContactLoadCellArrays, dinrail::IContactLoadCellArrays>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IContactLoadCellArrays,
                                           dinrail::IContactLoadCellArrays>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfContactLoadCellArrays() const override;
    yarp::dev::MAS_status getContactLoadCellArrayStatus(std::size_t index) const override;
    bool getContactLoadCellArrayName(std::size_t index, std::string& name) const override;
    std::size_t getContactLoadCellArraySize(std::size_t index) const override;
    bool getContactLoadCellArrayMeasure(std::size_t index,
                                        yarp::sig::Vector& output,
                                        double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::IEncoderArrays, dinrail::IEncoderArrays>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IEncoderArrays, dinrail::IEncoderArrays>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfEncoderArrays() const override;
    yarp::dev::MAS_status getEncoderArrayStatus(std::size_t index) const override;
    bool getEncoderArrayName(std::size_t index, std::string& name) const override;
    std::size_t getEncoderArraySize(std::size_t index) const override;
    bool getEncoderArrayMeasure(std::size_t index,
                                yarp::sig::Vector& output,
                                double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::ISkinPatches, dinrail::ISkinPatches>
    : public dinrail::InterfaceAdapterBase<yarp::dev::ISkinPatches, dinrail::ISkinPatches>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfSkinPatches() const override;
    yarp::dev::MAS_status getSkinPatchStatus(std::size_t index) const override;
    bool getSkinPatchName(std::size_t index, std::string& name) const override;
    std::size_t getSkinPatchSize(std::size_t index) const override;
    bool getSkinPatchMeasure(std::size_t index,
                             yarp::sig::Vector& output,
                             double& timestamp) const override;
};

template <>
class InterfaceAdapter<yarp::dev::ITemperatureSensors, dinrail::ITemperatureSensors>
    : public dinrail::InterfaceAdapterBase<yarp::dev::ITemperatureSensors,
                                           dinrail::ITemperatureSensors>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    std::size_t getNrOfTemperatureSensors() const override;
    yarp::dev::MAS_status getTemperatureSensorStatus(std::size_t index) const override;
    bool getTemperatureSensorName(std::size_t index, std::string& name) const override;
    bool getTemperatureSensorFrameName(std::size_t index, std::string& frame) const override;
    bool
    getTemperatureSensorMeasure(std::size_t index, double& output, double& timestamp) const override;
    bool getTemperatureSensorMeasure(std::size_t index,
                                     yarp::sig::Vector& output,
                                     double& timestamp) const override;
};

#define DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(CLASS, DINRAIL_INTERFACE, YARP_INTERFACE, COUNT, STATUS, NAME, FRAME, MEASURE) \
    template <> class InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE> final : public InterfaceAdapterBase<DINRAIL_INTERFACE, YARP_INTERFACE> \
    { public: explicit InterfaceAdapter(YARP_INTERFACE& source); std::size_t COUNT() const override; dinrail::MAS_status STATUS(std::size_t index) const override; bool NAME(std::size_t index, std::string& name) const override; bool FRAME(std::size_t index, std::string& frame) const override; bool MEASURE(std::size_t index, dinrail::VectorProxy<double>::Ref output, double& timestamp) const override; private: YARP_INTERFACE& m_source; };
DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailIThreeAxisGyroscopesFromYarpDevIThreeAxisGyroscopes,
    dinrail::IThreeAxisGyroscopes,
    yarp::dev::IThreeAxisGyroscopes,
    getNrOfThreeAxisGyroscopes,
    getThreeAxisGyroscopeStatus,
    getThreeAxisGyroscopeName,
    getThreeAxisGyroscopeFrameName,
    getThreeAxisGyroscopeMeasure)
DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailIThreeAxisLinearAccelerometersFromYarpDevIThreeAxisLinearAccelerometers,
    dinrail::IThreeAxisLinearAccelerometers,
    yarp::dev::IThreeAxisLinearAccelerometers,
    getNrOfThreeAxisLinearAccelerometers,
    getThreeAxisLinearAccelerometerStatus,
    getThreeAxisLinearAccelerometerName,
    getThreeAxisLinearAccelerometerFrameName,
    getThreeAxisLinearAccelerometerMeasure)
DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailIThreeAxisAngularAccelerometersFromYarpDevIThreeAxisAngularAccelerometers,
    dinrail::IThreeAxisAngularAccelerometers,
    yarp::dev::IThreeAxisAngularAccelerometers,
    getNrOfThreeAxisAngularAccelerometers,
    getThreeAxisAngularAccelerometerStatus,
    getThreeAxisAngularAccelerometerName,
    getThreeAxisAngularAccelerometerFrameName,
    getThreeAxisAngularAccelerometerMeasure)
DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailIThreeAxisMagnetometersFromYarpDevIThreeAxisMagnetometers,
    dinrail::IThreeAxisMagnetometers,
    yarp::dev::IThreeAxisMagnetometers,
    getNrOfThreeAxisMagnetometers,
    getThreeAxisMagnetometerStatus,
    getThreeAxisMagnetometerName,
    getThreeAxisMagnetometerFrameName,
    getThreeAxisMagnetometerMeasure)
DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(ProvideDinrailIPositionSensorsFromYarpDevIPositionSensors,
                                    dinrail::IPositionSensors,
                                    yarp::dev::IPositionSensors,
                                    getNrOfPositionSensors,
                                    getPositionSensorStatus,
                                    getPositionSensorName,
                                    getPositionSensorFrameName,
                                    getPositionSensorMeasure)
DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailILinearVelocitySensorsFromYarpDevILinearVelocitySensors,
    dinrail::ILinearVelocitySensors,
    yarp::dev::ILinearVelocitySensors,
    getNrOfLinearVelocitySensors,
    getLinearVelocitySensorStatus,
    getLinearVelocitySensorName,
    getLinearVelocitySensorFrameName,
    getLinearVelocitySensorMeasure)
DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(ProvideDinrailIOrientationSensorsFromYarpDevIOrientationSensors,
                                    dinrail::IOrientationSensors,
                                    yarp::dev::IOrientationSensors,
                                    getNrOfOrientationSensors,
                                    getOrientationSensorStatus,
                                    getOrientationSensorName,
                                    getOrientationSensorFrameName,
                                    getOrientationSensorMeasureAsRollPitchYaw)
DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailISixAxisForceTorqueSensorsFromYarpDevISixAxisForceTorqueSensors,
    dinrail::ISixAxisForceTorqueSensors,
    yarp::dev::ISixAxisForceTorqueSensors,
    getNrOfSixAxisForceTorqueSensors,
    getSixAxisForceTorqueSensorStatus,
    getSixAxisForceTorqueSensorName,
    getSixAxisForceTorqueSensorFrameName,
    getSixAxisForceTorqueSensorMeasure)
#undef DINRAIL_DECLARE_FRAME_SENSOR_ADAPTER

template <> class InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors> final : public InterfaceAdapterBase<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>
{ public: explicit InterfaceAdapter(yarp::dev::ITemperatureSensors& source); std::size_t getNrOfTemperatureSensors() const override; dinrail::MAS_status getTemperatureSensorStatus(std::size_t index) const override; bool getTemperatureSensorName(std::size_t index, std::string& name) const override; bool getTemperatureSensorFrameName(std::size_t index, std::string& frame) const override; bool getTemperatureSensorMeasure(std::size_t index, double& output, double& timestamp) const override; bool getTemperatureSensorMeasure(std::size_t index, dinrail::VectorProxy<double>::Ref output, double& timestamp) const override; private: yarp::dev::ITemperatureSensors& m_source; };

#define DINRAIL_DECLARE_ARRAY_SENSOR_ADAPTER(CLASS, DINRAIL_INTERFACE, YARP_INTERFACE, COUNT, STATUS, NAME, MEASURE, SIZE) \
    template <> class InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE> final : public InterfaceAdapterBase<DINRAIL_INTERFACE, YARP_INTERFACE> \
    { public: explicit InterfaceAdapter(YARP_INTERFACE& source); std::size_t COUNT() const override; dinrail::MAS_status STATUS(std::size_t index) const override; bool NAME(std::size_t index, std::string& name) const override; bool MEASURE(std::size_t index, dinrail::VectorProxy<double>::Ref output, double& timestamp) const override; std::size_t SIZE(std::size_t index) const override; private: YARP_INTERFACE& m_source; };
DINRAIL_DECLARE_ARRAY_SENSOR_ADAPTER(
    ProvideDinrailIContactLoadCellArraysFromYarpDevIContactLoadCellArrays,
    dinrail::IContactLoadCellArrays,
    yarp::dev::IContactLoadCellArrays,
    getNrOfContactLoadCellArrays,
    getContactLoadCellArrayStatus,
    getContactLoadCellArrayName,
    getContactLoadCellArrayMeasure,
    getContactLoadCellArraySize)
DINRAIL_DECLARE_ARRAY_SENSOR_ADAPTER(ProvideDinrailIEncoderArraysFromYarpDevIEncoderArrays,
                                    dinrail::IEncoderArrays,
                                    yarp::dev::IEncoderArrays,
                                    getNrOfEncoderArrays,
                                    getEncoderArrayStatus,
                                    getEncoderArrayName,
                                    getEncoderArrayMeasure,
                                    getEncoderArraySize)
DINRAIL_DECLARE_ARRAY_SENSOR_ADAPTER(ProvideDinrailISkinPatchesFromYarpDevISkinPatches,
                                    dinrail::ISkinPatches,
                                    yarp::dev::ISkinPatches,
                                    getNrOfSkinPatches,
                                    getSkinPatchStatus,
                                    getSkinPatchName,
                                    getSkinPatchMeasure,
                                    getSkinPatchSize)
#undef DINRAIL_DECLARE_ARRAY_SENSOR_ADAPTER

} // namespace dinrail

#endif
