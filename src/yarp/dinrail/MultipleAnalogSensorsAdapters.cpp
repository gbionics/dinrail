// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/MultipleAnalogSensorsAdapters.h>

#include <dinrail/MultipleAnalogSensorsInterfaces.h>

#include <yarp/dev/MultipleAnalogSensorsInterfaces.h>
#include <yarp/sig/Vector.h>

#include <algorithm>

namespace dinrail
{

std::size_t InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes,
                             dinrail::IThreeAxisGyroscopes>::getNrOfThreeAxisGyroscopes() const
{
    return source().getNrOfThreeAxisGyroscopes();
}

yarp::dev::MAS_status
InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes,
                 dinrail::IThreeAxisGyroscopes>::getThreeAxisGyroscopeStatus(std::size_t index) const
{
    return static_cast<yarp::dev::MAS_status>(source().getThreeAxisGyroscopeStatus(index));
}

bool InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes,
                      dinrail::IThreeAxisGyroscopes>::getThreeAxisGyroscopeName(std::size_t index,
                                                                                std::string& name)
    const
{
    return source().getThreeAxisGyroscopeName(index, name);
}

bool InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes, dinrail::IThreeAxisGyroscopes>::
    getThreeAxisGyroscopeFrameName(std::size_t index, std::string& frame) const
{
    return source().getThreeAxisGyroscopeFrameName(index, frame);
}

bool InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes, dinrail::IThreeAxisGyroscopes>::
    getThreeAxisGyroscopeMeasure(std::size_t index,
                                 yarp::sig::Vector& output,
                                 double& timestamp) const
{
    return source().getThreeAxisGyroscopeMeasure(index, output, timestamp);
}

std::size_t
InterfaceAdapter<yarp::dev::IPositionSensors, dinrail::IPositionSensors>::getNrOfPositionSensors()
    const
{
    return source().getNrOfPositionSensors();
}

yarp::dev::MAS_status
InterfaceAdapter<yarp::dev::IPositionSensors, dinrail::IPositionSensors>::getPositionSensorStatus(
    std::size_t index) const
{
    return static_cast<yarp::dev::MAS_status>(source().getPositionSensorStatus(index));
}

bool InterfaceAdapter<yarp::dev::IPositionSensors, dinrail::IPositionSensors>::getPositionSensorName(
    std::size_t index, std::string& name) const
{
    return source().getPositionSensorName(index, name);
}

bool InterfaceAdapter<yarp::dev::IPositionSensors,
                      dinrail::IPositionSensors>::getPositionSensorFrameName(std::size_t index,
                                                                             std::string& frame)
    const
{
    return source().getPositionSensorFrameName(index, frame);
}

bool InterfaceAdapter<yarp::dev::IPositionSensors,
                      dinrail::IPositionSensors>::getPositionSensorMeasure(std::size_t index,
                                                                           yarp::sig::Vector& output,
                                                                           double& timestamp) const
{
    return source().getPositionSensorMeasure(index, output, timestamp);
}

#define DINRAIL_DEFINE_YARP_FRAME_SENSOR_ADAPTER(YARP_INTERFACE,                     \
                                                 DINRAIL_INTERFACE,                  \
                                                 COUNT,                               \
                                                 STATUS,                              \
                                                 NAME,                                \
                                                 FRAME,                               \
                                                 MEASURE)                             \
    std::size_t InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::COUNT() const   \
    {                                                                                \
        return source().COUNT();                                                     \
    }                                                                                \
    yarp::dev::MAS_status                                                            \
    InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::STATUS(std::size_t index)  \
        const                                                                        \
    {                                                                                \
        return static_cast<yarp::dev::MAS_status>(source().STATUS(index));          \
    }                                                                                \
    bool InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::NAME(                 \
        std::size_t index, std::string& name) const                                  \
    {                                                                                \
        return source().NAME(index, name);                                           \
    }                                                                                \
    bool InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::FRAME(                \
        std::size_t index, std::string& frame) const                                 \
    {                                                                                \
        return source().FRAME(index, frame);                                         \
    }                                                                                \
    bool InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::MEASURE(              \
        std::size_t index, yarp::sig::Vector& output, double& timestamp) const      \
    {                                                                                \
        return source().MEASURE(index, output, timestamp);                          \
    }

DINRAIL_DEFINE_YARP_FRAME_SENSOR_ADAPTER(yarp::dev::IThreeAxisLinearAccelerometers,
                                         dinrail::IThreeAxisLinearAccelerometers,
                                         getNrOfThreeAxisLinearAccelerometers,
                                         getThreeAxisLinearAccelerometerStatus,
                                         getThreeAxisLinearAccelerometerName,
                                         getThreeAxisLinearAccelerometerFrameName,
                                         getThreeAxisLinearAccelerometerMeasure)
DINRAIL_DEFINE_YARP_FRAME_SENSOR_ADAPTER(yarp::dev::IThreeAxisAngularAccelerometers,
                                         dinrail::IThreeAxisAngularAccelerometers,
                                         getNrOfThreeAxisAngularAccelerometers,
                                         getThreeAxisAngularAccelerometerStatus,
                                         getThreeAxisAngularAccelerometerName,
                                         getThreeAxisAngularAccelerometerFrameName,
                                         getThreeAxisAngularAccelerometerMeasure)
DINRAIL_DEFINE_YARP_FRAME_SENSOR_ADAPTER(yarp::dev::IThreeAxisMagnetometers,
                                         dinrail::IThreeAxisMagnetometers,
                                         getNrOfThreeAxisMagnetometers,
                                         getThreeAxisMagnetometerStatus,
                                         getThreeAxisMagnetometerName,
                                         getThreeAxisMagnetometerFrameName,
                                         getThreeAxisMagnetometerMeasure)
DINRAIL_DEFINE_YARP_FRAME_SENSOR_ADAPTER(yarp::dev::ILinearVelocitySensors,
                                         dinrail::ILinearVelocitySensors,
                                         getNrOfLinearVelocitySensors,
                                         getLinearVelocitySensorStatus,
                                         getLinearVelocitySensorName,
                                         getLinearVelocitySensorFrameName,
                                         getLinearVelocitySensorMeasure)
DINRAIL_DEFINE_YARP_FRAME_SENSOR_ADAPTER(yarp::dev::IOrientationSensors,
                                         dinrail::IOrientationSensors,
                                         getNrOfOrientationSensors,
                                         getOrientationSensorStatus,
                                         getOrientationSensorName,
                                         getOrientationSensorFrameName,
                                         getOrientationSensorMeasureAsRollPitchYaw)
DINRAIL_DEFINE_YARP_FRAME_SENSOR_ADAPTER(yarp::dev::ISixAxisForceTorqueSensors,
                                         dinrail::ISixAxisForceTorqueSensors,
                                         getNrOfSixAxisForceTorqueSensors,
                                         getSixAxisForceTorqueSensorStatus,
                                         getSixAxisForceTorqueSensorName,
                                         getSixAxisForceTorqueSensorFrameName,
                                         getSixAxisForceTorqueSensorMeasure)

#undef DINRAIL_DEFINE_YARP_FRAME_SENSOR_ADAPTER

#define DINRAIL_DEFINE_YARP_ARRAY_SENSOR_ADAPTER(YARP_INTERFACE,                    \
                                                 DINRAIL_INTERFACE,                 \
                                                 COUNT,                              \
                                                 STATUS,                             \
                                                 NAME,                               \
                                                 MEASURE,                            \
                                                 SIZE)                               \
    std::size_t InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::COUNT() const  \
    {                                                                                \
        return source().COUNT();                                                     \
    }                                                                                \
    yarp::dev::MAS_status                                                            \
    InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::STATUS(std::size_t index)  \
        const                                                                        \
    {                                                                                \
        return static_cast<yarp::dev::MAS_status>(source().STATUS(index));          \
    }                                                                                \
    bool InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::NAME(                 \
        std::size_t index, std::string& name) const                                  \
    {                                                                                \
        return source().NAME(index, name);                                           \
    }                                                                                \
    bool InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::MEASURE(              \
        std::size_t index, yarp::sig::Vector& output, double& timestamp) const      \
    {                                                                                \
        return source().MEASURE(index, output, timestamp);                          \
    }                                                                                \
    std::size_t InterfaceAdapter<YARP_INTERFACE, DINRAIL_INTERFACE>::SIZE(          \
        std::size_t index) const                                                     \
    {                                                                                \
        return source().SIZE(index);                                                 \
    }

DINRAIL_DEFINE_YARP_ARRAY_SENSOR_ADAPTER(yarp::dev::IContactLoadCellArrays,
                                         dinrail::IContactLoadCellArrays,
                                         getNrOfContactLoadCellArrays,
                                         getContactLoadCellArrayStatus,
                                         getContactLoadCellArrayName,
                                         getContactLoadCellArrayMeasure,
                                         getContactLoadCellArraySize)
DINRAIL_DEFINE_YARP_ARRAY_SENSOR_ADAPTER(yarp::dev::IEncoderArrays,
                                         dinrail::IEncoderArrays,
                                         getNrOfEncoderArrays,
                                         getEncoderArrayStatus,
                                         getEncoderArrayName,
                                         getEncoderArrayMeasure,
                                         getEncoderArraySize)
DINRAIL_DEFINE_YARP_ARRAY_SENSOR_ADAPTER(yarp::dev::ISkinPatches,
                                         dinrail::ISkinPatches,
                                         getNrOfSkinPatches,
                                         getSkinPatchStatus,
                                         getSkinPatchName,
                                         getSkinPatchMeasure,
                                         getSkinPatchSize)

#undef DINRAIL_DEFINE_YARP_ARRAY_SENSOR_ADAPTER

std::size_t InterfaceAdapter<yarp::dev::ITemperatureSensors,
                             dinrail::ITemperatureSensors>::getNrOfTemperatureSensors() const
{
    return source().getNrOfTemperatureSensors();
}

yarp::dev::MAS_status
InterfaceAdapter<yarp::dev::ITemperatureSensors,
                 dinrail::ITemperatureSensors>::getTemperatureSensorStatus(std::size_t index) const
{
    return static_cast<yarp::dev::MAS_status>(source().getTemperatureSensorStatus(index));
}

bool InterfaceAdapter<yarp::dev::ITemperatureSensors,
                      dinrail::ITemperatureSensors>::getTemperatureSensorName(
    std::size_t index, std::string& name) const
{
    return source().getTemperatureSensorName(index, name);
}

bool InterfaceAdapter<yarp::dev::ITemperatureSensors,
                      dinrail::ITemperatureSensors>::getTemperatureSensorFrameName(
    std::size_t index, std::string& frame) const
{
    return source().getTemperatureSensorFrameName(index, frame);
}

bool InterfaceAdapter<yarp::dev::ITemperatureSensors,
                      dinrail::ITemperatureSensors>::getTemperatureSensorMeasure(
    std::size_t index, double& output, double& timestamp) const
{
    return source().getTemperatureSensorMeasure(index, output, timestamp);
}

bool InterfaceAdapter<yarp::dev::ITemperatureSensors,
                      dinrail::ITemperatureSensors>::getTemperatureSensorMeasure(
    std::size_t index, yarp::sig::Vector& output, double& timestamp) const
{
    return source().getTemperatureSensorMeasure(index, output, timestamp);
}

template <class Read> bool readYarpVector(dinrail::VectorProxy<double>::Ref output, Read&& read)
{
    yarp::sig::Vector yarpOutput;
    if (!read(yarpOutput))
    {
        return false;
    }

    const auto size = static_cast<dinrail::VectorProxy<double>::index_type>(yarpOutput.size());
    if (output.size() != size && !output.resizeVector(size))
    {
        return false;
    }
    std::copy(yarpOutput.begin(), yarpOutput.end(), output.begin());
    return true;
}

#define DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(CLASS, DINRAIL_INTERFACE, YARP_INTERFACE, COUNT, STATUS, NAME, FRAME, MEASURE) \
    InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::InterfaceAdapter(YARP_INTERFACE& source) : InterfaceAdapterBase(source), m_source(source) {} \
    std::size_t InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::COUNT() const { return m_source.COUNT(); } \
    dinrail::MAS_status InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::STATUS(std::size_t index) const { return static_cast<dinrail::MAS_status>(m_source.STATUS(index)); } \
    bool InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::NAME(std::size_t index, std::string& name) const { return m_source.NAME(index, name); } \
    bool InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::FRAME(std::size_t index, std::string& frame) const { return m_source.FRAME(index, frame); } \
    bool InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::MEASURE(std::size_t index, dinrail::VectorProxy<double>::Ref output, double& timestamp) const { return readYarpVector(output, [&](yarp::sig::Vector& yarpOutput) { return m_source.MEASURE(index, yarpOutput, timestamp); }); }

DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailIThreeAxisGyroscopesFromYarpDevIThreeAxisGyroscopes,
    dinrail::IThreeAxisGyroscopes,
    yarp::dev::IThreeAxisGyroscopes,
    getNrOfThreeAxisGyroscopes,
    getThreeAxisGyroscopeStatus,
    getThreeAxisGyroscopeName,
    getThreeAxisGyroscopeFrameName,
    getThreeAxisGyroscopeMeasure)
DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailIThreeAxisLinearAccelerometersFromYarpDevIThreeAxisLinearAccelerometers,
    dinrail::IThreeAxisLinearAccelerometers,
    yarp::dev::IThreeAxisLinearAccelerometers,
    getNrOfThreeAxisLinearAccelerometers,
    getThreeAxisLinearAccelerometerStatus,
    getThreeAxisLinearAccelerometerName,
    getThreeAxisLinearAccelerometerFrameName,
    getThreeAxisLinearAccelerometerMeasure)
DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailIThreeAxisAngularAccelerometersFromYarpDevIThreeAxisAngularAccelerometers,
    dinrail::IThreeAxisAngularAccelerometers,
    yarp::dev::IThreeAxisAngularAccelerometers,
    getNrOfThreeAxisAngularAccelerometers,
    getThreeAxisAngularAccelerometerStatus,
    getThreeAxisAngularAccelerometerName,
    getThreeAxisAngularAccelerometerFrameName,
    getThreeAxisAngularAccelerometerMeasure)
DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailIThreeAxisMagnetometersFromYarpDevIThreeAxisMagnetometers,
    dinrail::IThreeAxisMagnetometers,
    yarp::dev::IThreeAxisMagnetometers,
    getNrOfThreeAxisMagnetometers,
    getThreeAxisMagnetometerStatus,
    getThreeAxisMagnetometerName,
    getThreeAxisMagnetometerFrameName,
    getThreeAxisMagnetometerMeasure)
DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(ProvideDinrailIPositionSensorsFromYarpDevIPositionSensors,
                                    dinrail::IPositionSensors,
                                    yarp::dev::IPositionSensors,
                                    getNrOfPositionSensors,
                                    getPositionSensorStatus,
                                    getPositionSensorName,
                                    getPositionSensorFrameName,
                                    getPositionSensorMeasure)
DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailILinearVelocitySensorsFromYarpDevILinearVelocitySensors,
    dinrail::ILinearVelocitySensors,
    yarp::dev::ILinearVelocitySensors,
    getNrOfLinearVelocitySensors,
    getLinearVelocitySensorStatus,
    getLinearVelocitySensorName,
    getLinearVelocitySensorFrameName,
    getLinearVelocitySensorMeasure)
DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(ProvideDinrailIOrientationSensorsFromYarpDevIOrientationSensors,
                                    dinrail::IOrientationSensors,
                                    yarp::dev::IOrientationSensors,
                                    getNrOfOrientationSensors,
                                    getOrientationSensorStatus,
                                    getOrientationSensorName,
                                    getOrientationSensorFrameName,
                                    getOrientationSensorMeasureAsRollPitchYaw)
DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER(
    ProvideDinrailISixAxisForceTorqueSensorsFromYarpDevISixAxisForceTorqueSensors,
    dinrail::ISixAxisForceTorqueSensors,
    yarp::dev::ISixAxisForceTorqueSensors,
    getNrOfSixAxisForceTorqueSensors,
    getSixAxisForceTorqueSensorStatus,
    getSixAxisForceTorqueSensorName,
    getSixAxisForceTorqueSensorFrameName,
    getSixAxisForceTorqueSensorMeasure)

#undef DINRAIL_DEFINE_FRAME_SENSOR_ADAPTER

InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>::InterfaceAdapter(yarp::dev::ITemperatureSensors& source) : InterfaceAdapterBase(source), m_source(source) {}
std::size_t InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>::getNrOfTemperatureSensors() const { return m_source.getNrOfTemperatureSensors(); }
dinrail::MAS_status InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>::getTemperatureSensorStatus(std::size_t index) const { return static_cast<dinrail::MAS_status>(m_source.getTemperatureSensorStatus(index)); }
bool InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>::getTemperatureSensorName(std::size_t index, std::string& name) const { return m_source.getTemperatureSensorName(index, name); }
bool InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>::getTemperatureSensorFrameName(std::size_t index, std::string& frame) const { return m_source.getTemperatureSensorFrameName(index, frame); }
bool InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>::getTemperatureSensorMeasure(std::size_t index, double& output, double& timestamp) const { return m_source.getTemperatureSensorMeasure(index, output, timestamp); }
bool InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>::getTemperatureSensorMeasure(std::size_t index, dinrail::VectorProxy<double>::Ref output, double& timestamp) const { return readYarpVector(output, [&](yarp::sig::Vector& yarpOutput) { return m_source.getTemperatureSensorMeasure(index, yarpOutput, timestamp); }); }


#define DINRAIL_DEFINE_ARRAY_SENSOR_ADAPTER(CLASS, DINRAIL_INTERFACE, YARP_INTERFACE, COUNT, STATUS, NAME, MEASURE, SIZE) \
    InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::InterfaceAdapter(YARP_INTERFACE& source) : InterfaceAdapterBase(source), m_source(source) {} \
    std::size_t InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::COUNT() const { return m_source.COUNT(); } \
    dinrail::MAS_status InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::STATUS(std::size_t index) const { return static_cast<dinrail::MAS_status>(m_source.STATUS(index)); } \
    bool InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::NAME(std::size_t index, std::string& name) const { return m_source.NAME(index, name); } \
    bool InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::MEASURE(std::size_t index, dinrail::VectorProxy<double>::Ref output, double& timestamp) const { return readYarpVector(output, [&](yarp::sig::Vector& yarpOutput) { return m_source.MEASURE(index, yarpOutput, timestamp); }); } \
    std::size_t InterfaceAdapter<DINRAIL_INTERFACE, YARP_INTERFACE>::SIZE(std::size_t index) const { return m_source.SIZE(index); }

DINRAIL_DEFINE_ARRAY_SENSOR_ADAPTER(
    ProvideDinrailIContactLoadCellArraysFromYarpDevIContactLoadCellArrays,
    dinrail::IContactLoadCellArrays,
    yarp::dev::IContactLoadCellArrays,
    getNrOfContactLoadCellArrays,
    getContactLoadCellArrayStatus,
    getContactLoadCellArrayName,
    getContactLoadCellArrayMeasure,
    getContactLoadCellArraySize)
DINRAIL_DEFINE_ARRAY_SENSOR_ADAPTER(ProvideDinrailIEncoderArraysFromYarpDevIEncoderArrays,
                                    dinrail::IEncoderArrays,
                                    yarp::dev::IEncoderArrays,
                                    getNrOfEncoderArrays,
                                    getEncoderArrayStatus,
                                    getEncoderArrayName,
                                    getEncoderArrayMeasure,
                                    getEncoderArraySize)
DINRAIL_DEFINE_ARRAY_SENSOR_ADAPTER(ProvideDinrailISkinPatchesFromYarpDevISkinPatches,
                                    dinrail::ISkinPatches,
                                    yarp::dev::ISkinPatches,
                                    getNrOfSkinPatches,
                                    getSkinPatchStatus,
                                    getSkinPatchName,
                                    getSkinPatchMeasure,
                                    getSkinPatchSize)

#undef DINRAIL_DEFINE_ARRAY_SENSOR_ADAPTER

} // namespace dinrail
