// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DR_MULTIPLE_ANALOG_SENSORS_FAKE_H
#define DINRAIL_DR_MULTIPLE_ANALOG_SENSORS_FAKE_H

#include <dinrail/IDevice.h>
#include <dinrail/MultipleAnalogSensorsInterfaces.h>

#include <mutex>
#include <string>
#include <vector>

namespace dinrail
{

/**
 * @brief Fake multiple analog sensors device driven through simulation interfaces.
 *
 * By default the device exposes one sensor for every YARP multiple analog
 * sensor family. The number of sensors can be changed with
 * `number_of_sensors`, or with a family-specific `number_of_*` parameter.
 *
 * Sensor metadata is configured when the device is opened. For every family,
 * the optional `<family>_names` parameter is a vector of one name per sensor.
 * Families with frame-name accessors additionally accept
 * `<family>_frame_names`. The supported family prefixes are
 * `three_axis_gyroscope`, `three_axis_linear_accelerometer`,
 * `three_axis_angular_accelerometer`, `three_axis_magnetometer`,
 * `position_sensor`, `linear_velocity_sensor`, `orientation_sensor`,
 * `temperature_sensor`, `six_axis_force_torque_sensor`,
 * `contact_load_cell_array`, `encoder_array`, and `skin_patch`.
 *
 * A supplied metadata vector must have the same size as its sensor family.
 * Simulation interfaces deliberately do not modify this static metadata.
 */
class DrMultipleAnalogSensorsFake final : public IDevice,
                                          public IThreeAxisGyroscopes,
                                          public IThreeAxisGyroscopesSimulation,
                                          public IThreeAxisLinearAccelerometers,
                                          public IThreeAxisLinearAccelerometersSimulation,
                                          public IThreeAxisAngularAccelerometers,
                                          public IThreeAxisAngularAccelerometersSimulation,
                                          public IThreeAxisMagnetometers,
                                          public IThreeAxisMagnetometersSimulation,
                                          public IPositionSensors,
                                          public IPositionSensorsSimulation,
                                          public ILinearVelocitySensors,
                                          public ILinearVelocitySensorsSimulation,
                                          public IOrientationSensors,
                                          public IOrientationSensorsSimulation,
                                          public ITemperatureSensors,
                                          public ITemperatureSensorsSimulation,
                                          public ISixAxisForceTorqueSensors,
                                          public ISixAxisForceTorqueSensorsSimulation,
                                          public IContactLoadCellArrays,
                                          public IContactLoadCellArraysSimulation,
                                          public IEncoderArrays,
                                          public IEncoderArraysSimulation,
                                          public ISkinPatches,
                                          public ISkinPatchesSimulation
{
public:
    DrMultipleAnalogSensorsFake() = default;
    ~DrMultipleAnalogSensorsFake() override = default;

    bool open(const Parameters& config) override;
    bool close() override;

    std::size_t getNrOfThreeAxisGyroscopes() const override;
    MAS_status getThreeAxisGyroscopeStatus(std::size_t index) const override;
    bool getThreeAxisGyroscopeName(std::size_t index, std::string& name) const override;
    bool getThreeAxisGyroscopeFrameName(std::size_t index, std::string& frameName) const override;
    bool getThreeAxisGyroscopeMeasure(std::size_t index,
                                      VectorProxy<double>::Ref out,
                                      double& timestamp) const override;
    bool setThreeAxisGyroscopeStatus(std::size_t index, MAS_status status) override;
    bool setThreeAxisGyroscopeMeasure(std::size_t index,
                                      const VectorProxy<const double>::Ref measure,
                                      double timestamp) override;

    std::size_t getNrOfThreeAxisLinearAccelerometers() const override;
    MAS_status getThreeAxisLinearAccelerometerStatus(std::size_t index) const override;
    bool getThreeAxisLinearAccelerometerName(std::size_t index, std::string& name) const override;
    bool getThreeAxisLinearAccelerometerFrameName(std::size_t index,
                                                  std::string& frameName) const override;
    bool getThreeAxisLinearAccelerometerMeasure(std::size_t index,
                                                VectorProxy<double>::Ref out,
                                                double& timestamp) const override;
    bool setThreeAxisLinearAccelerometerStatus(std::size_t index, MAS_status status) override;
    bool setThreeAxisLinearAccelerometerMeasure(std::size_t index,
                                                const VectorProxy<const double>::Ref measure,
                                                double timestamp) override;

    std::size_t getNrOfThreeAxisAngularAccelerometers() const override;
    MAS_status getThreeAxisAngularAccelerometerStatus(std::size_t index) const override;
    bool getThreeAxisAngularAccelerometerName(std::size_t index, std::string& name) const override;
    bool getThreeAxisAngularAccelerometerFrameName(std::size_t index,
                                                   std::string& frameName) const override;
    bool getThreeAxisAngularAccelerometerMeasure(std::size_t index,
                                                 VectorProxy<double>::Ref out,
                                                 double& timestamp) const override;
    bool setThreeAxisAngularAccelerometerStatus(std::size_t index, MAS_status status) override;
    bool setThreeAxisAngularAccelerometerMeasure(std::size_t index,
                                                 const VectorProxy<const double>::Ref measure,
                                                 double timestamp) override;

    std::size_t getNrOfThreeAxisMagnetometers() const override;
    MAS_status getThreeAxisMagnetometerStatus(std::size_t index) const override;
    bool getThreeAxisMagnetometerName(std::size_t index, std::string& name) const override;
    bool
    getThreeAxisMagnetometerFrameName(std::size_t index, std::string& frameName) const override;
    bool getThreeAxisMagnetometerMeasure(std::size_t index,
                                         VectorProxy<double>::Ref out,
                                         double& timestamp) const override;
    bool setThreeAxisMagnetometerStatus(std::size_t index, MAS_status status) override;
    bool setThreeAxisMagnetometerMeasure(std::size_t index,
                                         const VectorProxy<const double>::Ref measure,
                                         double timestamp) override;

    std::size_t getNrOfPositionSensors() const override;
    MAS_status getPositionSensorStatus(std::size_t index) const override;
    bool getPositionSensorName(std::size_t index, std::string& name) const override;
    bool getPositionSensorFrameName(std::size_t index, std::string& frameName) const override;
    bool getPositionSensorMeasure(std::size_t index,
                                  VectorProxy<double>::Ref xyz,
                                  double& timestamp) const override;
    bool setPositionSensorStatus(std::size_t index, MAS_status status) override;
    bool setPositionSensorMeasure(std::size_t index,
                                  const VectorProxy<const double>::Ref measure,
                                  double timestamp) override;

    std::size_t getNrOfLinearVelocitySensors() const override;
    MAS_status getLinearVelocitySensorStatus(std::size_t index) const override;
    bool getLinearVelocitySensorName(std::size_t index, std::string& name) const override;
    bool getLinearVelocitySensorFrameName(std::size_t index, std::string& frameName) const override;
    bool getLinearVelocitySensorMeasure(std::size_t index,
                                        VectorProxy<double>::Ref xyz,
                                        double& timestamp) const override;
    bool setLinearVelocitySensorStatus(std::size_t index, MAS_status status) override;
    bool setLinearVelocitySensorMeasure(std::size_t index,
                                        const VectorProxy<const double>::Ref measure,
                                        double timestamp) override;

    std::size_t getNrOfOrientationSensors() const override;
    MAS_status getOrientationSensorStatus(std::size_t index) const override;
    bool getOrientationSensorName(std::size_t index, std::string& name) const override;
    bool getOrientationSensorFrameName(std::size_t index, std::string& frameName) const override;
    bool getOrientationSensorMeasureAsRollPitchYaw(std::size_t index,
                                                   VectorProxy<double>::Ref rpy,
                                                   double& timestamp) const override;
    bool setOrientationSensorStatus(std::size_t index, MAS_status status) override;
    bool setOrientationSensorMeasureAsRollPitchYaw(std::size_t index,
                                                   const VectorProxy<const double>::Ref rpy,
                                                   double timestamp) override;

    std::size_t getNrOfTemperatureSensors() const override;
    MAS_status getTemperatureSensorStatus(std::size_t index) const override;
    bool getTemperatureSensorName(std::size_t index, std::string& name) const override;
    bool getTemperatureSensorFrameName(std::size_t index, std::string& frameName) const override;
    bool
    getTemperatureSensorMeasure(std::size_t index, double& out, double& timestamp) const override;
    bool getTemperatureSensorMeasure(std::size_t index,
                                     VectorProxy<double>::Ref out,
                                     double& timestamp) const override;
    bool setTemperatureSensorStatus(std::size_t index, MAS_status status) override;
    bool setTemperatureSensorMeasure(std::size_t index, double measure, double timestamp) override;

    std::size_t getNrOfSixAxisForceTorqueSensors() const override;
    MAS_status getSixAxisForceTorqueSensorStatus(std::size_t index) const override;
    bool getSixAxisForceTorqueSensorName(std::size_t index, std::string& name) const override;
    bool
    getSixAxisForceTorqueSensorFrameName(std::size_t index, std::string& frameName) const override;
    bool getSixAxisForceTorqueSensorMeasure(std::size_t index,
                                            VectorProxy<double>::Ref out,
                                            double& timestamp) const override;
    bool setSixAxisForceTorqueSensorStatus(std::size_t index, MAS_status status) override;
    bool setSixAxisForceTorqueSensorMeasure(std::size_t index,
                                            const VectorProxy<const double>::Ref measure,
                                            double timestamp) override;

    std::size_t getNrOfContactLoadCellArrays() const override;
    MAS_status getContactLoadCellArrayStatus(std::size_t index) const override;
    bool getContactLoadCellArrayName(std::size_t index, std::string& name) const override;
    bool getContactLoadCellArrayMeasure(std::size_t index,
                                        VectorProxy<double>::Ref out,
                                        double& timestamp) const override;
    std::size_t getContactLoadCellArraySize(std::size_t index) const override;
    bool setContactLoadCellArrayStatus(std::size_t index, MAS_status status) override;
    bool setContactLoadCellArrayMeasure(std::size_t index,
                                        const VectorProxy<const double>::Ref measure,
                                        double timestamp) override;

    std::size_t getNrOfEncoderArrays() const override;
    MAS_status getEncoderArrayStatus(std::size_t index) const override;
    bool getEncoderArrayName(std::size_t index, std::string& name) const override;
    bool getEncoderArrayMeasure(std::size_t index,
                                VectorProxy<double>::Ref out,
                                double& timestamp) const override;
    std::size_t getEncoderArraySize(std::size_t index) const override;
    bool setEncoderArrayStatus(std::size_t index, MAS_status status) override;
    bool setEncoderArrayMeasure(std::size_t index,
                                const VectorProxy<const double>::Ref measure,
                                double timestamp) override;

    std::size_t getNrOfSkinPatches() const override;
    MAS_status getSkinPatchStatus(std::size_t index) const override;
    bool getSkinPatchName(std::size_t index, std::string& name) const override;
    bool getSkinPatchMeasure(std::size_t index,
                             VectorProxy<double>::Ref out,
                             double& timestamp) const override;
    std::size_t getSkinPatchSize(std::size_t index) const override;
    bool setSkinPatchStatus(std::size_t index, MAS_status status) override;
    bool setSkinPatchMeasure(std::size_t index,
                             const VectorProxy<const double>::Ref measure,
                             double timestamp) override;

public:
    struct Sensor
    {
        MAS_status status{MAS_OK};
        std::string name;
        std::string frameName;
        std::vector<double> measure;
        double timestamp{0.0};
    };

private:
    mutable std::mutex m_mutex;
    bool m_opened{false};
    std::vector<Sensor> m_gyroscopes;
    std::vector<Sensor> m_linearAccelerometers;
    std::vector<Sensor> m_angularAccelerometers;
    std::vector<Sensor> m_magnetometers;
    std::vector<Sensor> m_positions;
    std::vector<Sensor> m_linearVelocities;
    std::vector<Sensor> m_orientations;
    std::vector<Sensor> m_temperatures;
    std::vector<Sensor> m_forceTorques;
    std::vector<Sensor> m_contactLoadCells;
    std::vector<Sensor> m_encoderArrays;
    std::vector<Sensor> m_skinPatches;
};

} // namespace dinrail

#endif // DINRAIL_DR_MULTIPLE_ANALOG_SENSORS_FAKE_H
