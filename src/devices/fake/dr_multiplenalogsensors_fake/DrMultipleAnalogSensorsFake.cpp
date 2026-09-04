// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "DrMultipleAnalogSensorsFake.h"

#include <algorithm>

namespace dinrail
{

namespace
{

using Sensor = DrMultipleAnalogSensorsFake::Sensor;

bool validIndex(const std::vector<Sensor>& sensors, std::size_t index)
{
    return index < sensors.size();
}

bool readMeasure(const Sensor& sensor, VectorProxy<double>::Ref output, double& timestamp)
{
    if (output.size() != static_cast<VectorProxy<double>::index_type>(sensor.measure.size())
        && !output.resizeVector(
            static_cast<VectorProxy<double>::index_type>(sensor.measure.size())))
    {
        return false;
    }

    std::copy(sensor.measure.begin(), sensor.measure.end(), output.begin());
    timestamp = sensor.timestamp;
    return true;
}

bool writeMeasure(Sensor& sensor, const VectorProxy<const double>::Ref measure, double timestamp)
{
    if (measure.size() != static_cast<VectorProxy<const double>::index_type>(sensor.measure.size()))
    {
        return false;
    }

    std::copy(measure.begin(), measure.end(), sensor.measure.begin());
    sensor.timestamp = timestamp;
    return true;
}

int sensorCount(const Parameters& config, const char* specificKey)
{
    int count = 1;
    config.getParameter("number_of_sensors", count);
    config.getParameter(specificKey, count);
    return count;
}

std::vector<Sensor> makeSensors(const Parameters& config,
                                const char* specificKey,
                                const char* namePrefix,
                                std::size_t measureSize)
{
    const int count = sensorCount(config, specificKey);
    if (count < 0)
    {
        return {};
    }

    std::vector<Sensor> sensors(static_cast<std::size_t>(count));
    for (std::size_t index = 0; index < sensors.size(); ++index)
    {
        sensors[index].name = std::string(namePrefix) + std::to_string(index);
        sensors[index].frameName = sensors[index].name + "_frame";
        sensors[index].measure.assign(measureSize, 0.0);
    }
    return sensors;
}

bool configureMetadata(const Parameters& config,
                       std::vector<Sensor>& sensors,
                       const char* namesKey,
                       const char* frameNamesKey = nullptr)
{
    if (config.check<std::vector<std::string>>(namesKey))
    {
        const auto& names = config.find(namesKey).as<std::vector<std::string>>();
        if (names.size() != sensors.size())
        {
            return false;
        }
        for (std::size_t index = 0; index < sensors.size(); ++index)
        {
            sensors[index].name = names[index];
        }
    }

    if (frameNamesKey != nullptr && config.check<std::vector<std::string>>(frameNamesKey))
    {
        const auto& frameNames = config.find(frameNamesKey).as<std::vector<std::string>>();
        if (frameNames.size() != sensors.size())
        {
            return false;
        }
        for (std::size_t index = 0; index < sensors.size(); ++index)
        {
            sensors[index].frameName = frameNames[index];
        }
    }

    return true;
}

} // namespace

bool DrMultipleAnalogSensorsFake::open(const Parameters& config)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_opened = false;

    m_gyroscopes = makeSensors(config, "number_of_three_axis_gyroscopes", "gyroscope", 3);
    m_linearAccelerometers = makeSensors(config,
                                         "number_of_three_axis_linear_accelerometers",
                                         "linear_accelerometer",
                                         3);
    m_angularAccelerometers = makeSensors(config,
                                          "number_of_three_axis_angular_accelerometers",
                                          "angular_accelerometer",
                                          3);
    m_magnetometers = makeSensors(config, "number_of_three_axis_magnetometers", "magnetometer", 3);
    m_positions = makeSensors(config, "number_of_position_sensors", "position_sensor", 3);
    m_linearVelocities
        = makeSensors(config, "number_of_linear_velocity_sensors", "linear_velocity", 3);
    m_orientations = makeSensors(config, "number_of_orientation_sensors", "orientation_sensor", 3);
    m_temperatures = makeSensors(config, "number_of_temperature_sensors", "temperature_sensor", 1);
    m_forceTorques
        = makeSensors(config, "number_of_six_axis_force_torque_sensors", "force_torque", 6);
    m_contactLoadCells
        = makeSensors(config, "number_of_contact_load_cell_arrays", "contact_load_cell", 4);
    m_encoderArrays = makeSensors(config, "number_of_encoder_arrays", "encoder_array", 4);
    m_skinPatches = makeSensors(config, "number_of_skin_patches", "skin_patch", 4);

    if (!configureMetadata(config,
                           m_gyroscopes,
                           "three_axis_gyroscope_names",
                           "three_axis_gyroscope_frame_names")
        || !configureMetadata(config,
                              m_linearAccelerometers,
                              "three_axis_linear_accelerometer_names",
                              "three_axis_linear_accelerometer_frame_names")
        || !configureMetadata(config,
                              m_angularAccelerometers,
                              "three_axis_angular_accelerometer_names",
                              "three_axis_angular_accelerometer_frame_names")
        || !configureMetadata(config,
                              m_magnetometers,
                              "three_axis_magnetometer_names",
                              "three_axis_magnetometer_frame_names")
        || !configureMetadata(config,
                              m_positions,
                              "position_sensor_names",
                              "position_sensor_frame_names")
        || !configureMetadata(config,
                              m_linearVelocities,
                              "linear_velocity_sensor_names",
                              "linear_velocity_sensor_frame_names")
        || !configureMetadata(config,
                              m_orientations,
                              "orientation_sensor_names",
                              "orientation_sensor_frame_names")
        || !configureMetadata(config,
                              m_temperatures,
                              "temperature_sensor_names",
                              "temperature_sensor_frame_names")
        || !configureMetadata(config,
                              m_forceTorques,
                              "six_axis_force_torque_sensor_names",
                              "six_axis_force_torque_sensor_frame_names")
        || !configureMetadata(config, m_contactLoadCells, "contact_load_cell_array_names")
        || !configureMetadata(config, m_encoderArrays, "encoder_array_names")
        || !configureMetadata(config, m_skinPatches, "skin_patch_names"))
    {
        return false;
    }

    // A negative family-specific count is invalid. Empty is otherwise a valid
    // configuration and is useful when a simulator only needs selected families.
    const char* countKeys[] = {"number_of_sensors",
                               "number_of_three_axis_gyroscopes",
                               "number_of_three_axis_linear_accelerometers",
                               "number_of_three_axis_angular_accelerometers",
                               "number_of_three_axis_magnetometers",
                               "number_of_position_sensors",
                               "number_of_linear_velocity_sensors",
                               "number_of_orientation_sensors",
                               "number_of_temperature_sensors",
                               "number_of_six_axis_force_torque_sensors",
                               "number_of_contact_load_cell_arrays",
                               "number_of_encoder_arrays",
                               "number_of_skin_patches"};
    m_opened = true;
    for (const char* key : countKeys)
    {
        int count = 0;
        if (config.getParameter(key, count) && count < 0)
        {
            m_opened = false;
        }
    }
    return m_opened;
}

bool DrMultipleAnalogSensorsFake::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_opened = false;
    return true;
}

#define DEFINE_FRAME_SENSOR_METHODS(PREFIX,                                                        \
                                    COUNT,                                                         \
                                    STATUS_GET,                                                    \
                                    NAME_GET,                                                      \
                                    FRAME_GET,                                                     \
                                    MEASURE_GET,                                                   \
                                    STATUS_SET,                                                    \
                                    MEASURE_SET)                                                   \
    std::size_t DrMultipleAnalogSensorsFake::COUNT() const                                         \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return m_##PREFIX.size();                                                                  \
    }                                                                                              \
    MAS_status DrMultipleAnalogSensorsFake::STATUS_GET(std::size_t index) const                    \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return validIndex(m_##PREFIX, index) ? m_##PREFIX[index].status : MAS_UNKNOWN;             \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::NAME_GET(std::size_t index, std::string& name) const         \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        if (!validIndex(m_##PREFIX, index))                                                        \
            return false;                                                                          \
        name = m_##PREFIX[index].name;                                                             \
        return true;                                                                               \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::FRAME_GET(std::size_t index, std::string& frameName) const   \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        if (!validIndex(m_##PREFIX, index))                                                        \
            return false;                                                                          \
        frameName = m_##PREFIX[index].frameName;                                                   \
        return true;                                                                               \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::MEASURE_GET(std::size_t index,                               \
                                                  VectorProxy<double>::Ref output,                 \
                                                  double& timestamp) const                         \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return validIndex(m_##PREFIX, index) && readMeasure(m_##PREFIX[index], output, timestamp); \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::STATUS_SET(std::size_t index, MAS_status status)             \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        if (!validIndex(m_##PREFIX, index))                                                        \
            return false;                                                                          \
        m_##PREFIX[index].status = status;                                                         \
        return true;                                                                               \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::MEASURE_SET(std::size_t index,                               \
                                                  const VectorProxy<const double>::Ref measure,    \
                                                  double timestamp)                                \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return validIndex(m_##PREFIX, index)                                                       \
               && writeMeasure(m_##PREFIX[index], measure, timestamp);                             \
    }

DEFINE_FRAME_SENSOR_METHODS(gyroscopes,
                            getNrOfThreeAxisGyroscopes,
                            getThreeAxisGyroscopeStatus,
                            getThreeAxisGyroscopeName,
                            getThreeAxisGyroscopeFrameName,
                            getThreeAxisGyroscopeMeasure,
                            setThreeAxisGyroscopeStatus,
                            setThreeAxisGyroscopeMeasure)
DEFINE_FRAME_SENSOR_METHODS(linearAccelerometers,
                            getNrOfThreeAxisLinearAccelerometers,
                            getThreeAxisLinearAccelerometerStatus,
                            getThreeAxisLinearAccelerometerName,
                            getThreeAxisLinearAccelerometerFrameName,
                            getThreeAxisLinearAccelerometerMeasure,
                            setThreeAxisLinearAccelerometerStatus,
                            setThreeAxisLinearAccelerometerMeasure)
DEFINE_FRAME_SENSOR_METHODS(angularAccelerometers,
                            getNrOfThreeAxisAngularAccelerometers,
                            getThreeAxisAngularAccelerometerStatus,
                            getThreeAxisAngularAccelerometerName,
                            getThreeAxisAngularAccelerometerFrameName,
                            getThreeAxisAngularAccelerometerMeasure,
                            setThreeAxisAngularAccelerometerStatus,
                            setThreeAxisAngularAccelerometerMeasure)
DEFINE_FRAME_SENSOR_METHODS(magnetometers,
                            getNrOfThreeAxisMagnetometers,
                            getThreeAxisMagnetometerStatus,
                            getThreeAxisMagnetometerName,
                            getThreeAxisMagnetometerFrameName,
                            getThreeAxisMagnetometerMeasure,
                            setThreeAxisMagnetometerStatus,
                            setThreeAxisMagnetometerMeasure)
DEFINE_FRAME_SENSOR_METHODS(positions,
                            getNrOfPositionSensors,
                            getPositionSensorStatus,
                            getPositionSensorName,
                            getPositionSensorFrameName,
                            getPositionSensorMeasure,
                            setPositionSensorStatus,
                            setPositionSensorMeasure)
DEFINE_FRAME_SENSOR_METHODS(linearVelocities,
                            getNrOfLinearVelocitySensors,
                            getLinearVelocitySensorStatus,
                            getLinearVelocitySensorName,
                            getLinearVelocitySensorFrameName,
                            getLinearVelocitySensorMeasure,
                            setLinearVelocitySensorStatus,
                            setLinearVelocitySensorMeasure)
DEFINE_FRAME_SENSOR_METHODS(orientations,
                            getNrOfOrientationSensors,
                            getOrientationSensorStatus,
                            getOrientationSensorName,
                            getOrientationSensorFrameName,
                            getOrientationSensorMeasureAsRollPitchYaw,
                            setOrientationSensorStatus,
                            setOrientationSensorMeasureAsRollPitchYaw)
DEFINE_FRAME_SENSOR_METHODS(forceTorques,
                            getNrOfSixAxisForceTorqueSensors,
                            getSixAxisForceTorqueSensorStatus,
                            getSixAxisForceTorqueSensorName,
                            getSixAxisForceTorqueSensorFrameName,
                            getSixAxisForceTorqueSensorMeasure,
                            setSixAxisForceTorqueSensorStatus,
                            setSixAxisForceTorqueSensorMeasure)

#undef DEFINE_FRAME_SENSOR_METHODS

#define DEFINE_ARRAY_SENSOR_METHODS(PREFIX,                                                        \
                                    COUNT,                                                         \
                                    STATUS_GET,                                                    \
                                    NAME_GET,                                                      \
                                    MEASURE_GET,                                                   \
                                    SIZE_GET,                                                      \
                                    STATUS_SET,                                                    \
                                    MEASURE_SET)                                                   \
    std::size_t DrMultipleAnalogSensorsFake::COUNT() const                                         \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return m_##PREFIX.size();                                                                  \
    }                                                                                              \
    MAS_status DrMultipleAnalogSensorsFake::STATUS_GET(std::size_t index) const                    \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return validIndex(m_##PREFIX, index) ? m_##PREFIX[index].status : MAS_UNKNOWN;             \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::NAME_GET(std::size_t index, std::string& name) const         \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        if (!validIndex(m_##PREFIX, index))                                                        \
            return false;                                                                          \
        name = m_##PREFIX[index].name;                                                             \
        return true;                                                                               \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::MEASURE_GET(std::size_t index,                               \
                                                  VectorProxy<double>::Ref output,                 \
                                                  double& timestamp) const                         \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return validIndex(m_##PREFIX, index) && readMeasure(m_##PREFIX[index], output, timestamp); \
    }                                                                                              \
    std::size_t DrMultipleAnalogSensorsFake::SIZE_GET(std::size_t index) const                     \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return validIndex(m_##PREFIX, index) ? m_##PREFIX[index].measure.size() : 0;               \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::STATUS_SET(std::size_t index, MAS_status status)             \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        if (!validIndex(m_##PREFIX, index))                                                        \
            return false;                                                                          \
        m_##PREFIX[index].status = status;                                                         \
        return true;                                                                               \
    }                                                                                              \
    bool DrMultipleAnalogSensorsFake::MEASURE_SET(std::size_t index,                               \
                                                  const VectorProxy<const double>::Ref measure,    \
                                                  double timestamp)                                \
    {                                                                                              \
        std::lock_guard<std::mutex> lock(m_mutex);                                                 \
        return validIndex(m_##PREFIX, index)                                                       \
               && writeMeasure(m_##PREFIX[index], measure, timestamp);                             \
    }

DEFINE_ARRAY_SENSOR_METHODS(contactLoadCells,
                            getNrOfContactLoadCellArrays,
                            getContactLoadCellArrayStatus,
                            getContactLoadCellArrayName,
                            getContactLoadCellArrayMeasure,
                            getContactLoadCellArraySize,
                            setContactLoadCellArrayStatus,
                            setContactLoadCellArrayMeasure)
DEFINE_ARRAY_SENSOR_METHODS(encoderArrays,
                            getNrOfEncoderArrays,
                            getEncoderArrayStatus,
                            getEncoderArrayName,
                            getEncoderArrayMeasure,
                            getEncoderArraySize,
                            setEncoderArrayStatus,
                            setEncoderArrayMeasure)
DEFINE_ARRAY_SENSOR_METHODS(skinPatches,
                            getNrOfSkinPatches,
                            getSkinPatchStatus,
                            getSkinPatchName,
                            getSkinPatchMeasure,
                            getSkinPatchSize,
                            setSkinPatchStatus,
                            setSkinPatchMeasure)

#undef DEFINE_ARRAY_SENSOR_METHODS

std::size_t DrMultipleAnalogSensorsFake::getNrOfTemperatureSensors() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_temperatures.size();
}

MAS_status DrMultipleAnalogSensorsFake::getTemperatureSensorStatus(std::size_t index) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return validIndex(m_temperatures, index) ? m_temperatures[index].status : MAS_UNKNOWN;
}

bool DrMultipleAnalogSensorsFake::getTemperatureSensorName(std::size_t index,
                                                           std::string& name) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!validIndex(m_temperatures, index))
        return false;
    name = m_temperatures[index].name;
    return true;
}

bool DrMultipleAnalogSensorsFake::getTemperatureSensorFrameName(std::size_t index,
                                                                std::string& frameName) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!validIndex(m_temperatures, index))
        return false;
    frameName = m_temperatures[index].frameName;
    return true;
}

bool DrMultipleAnalogSensorsFake::getTemperatureSensorMeasure(std::size_t index,
                                                              double& output,
                                                              double& timestamp) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!validIndex(m_temperatures, index))
        return false;
    output = m_temperatures[index].measure.front();
    timestamp = m_temperatures[index].timestamp;
    return true;
}

bool DrMultipleAnalogSensorsFake::getTemperatureSensorMeasure(std::size_t index,
                                                              VectorProxy<double>::Ref output,
                                                              double& timestamp) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return validIndex(m_temperatures, index)
           && readMeasure(m_temperatures[index], output, timestamp);
}

bool DrMultipleAnalogSensorsFake::setTemperatureSensorStatus(std::size_t index, MAS_status status)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!validIndex(m_temperatures, index))
        return false;
    m_temperatures[index].status = status;
    return true;
}

bool DrMultipleAnalogSensorsFake::setTemperatureSensorMeasure(std::size_t index,
                                                              double measure,
                                                              double timestamp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!validIndex(m_temperatures, index))
        return false;
    m_temperatures[index].measure[0] = measure;
    m_temperatures[index].timestamp = timestamp;
    return true;
}

} // namespace dinrail
