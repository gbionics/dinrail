// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_MULTIPLE_ANALOG_SENSORS_INTERFACES_H
#define DINRAIL_MULTIPLE_ANALOG_SENSORS_INTERFACES_H

#include <dinrail/VectorProxy.h>

#include <cstddef>
#include <string>

namespace dinrail
{

/**
 * @brief Status of an individual sensor in a multiple analog sensors device.
 *
 * The numeric values intentionally match `yarp::dev::MAS_status`, allowing a
 * direct translation when adapting a YARP device.
 */
enum MAS_status
{
    MAS_OK = 0, ///< The sensor is working correctly.
    MAS_ERROR = 1, ///< The sensor is in a generic error state.
    MAS_OVF = 2, ///< The sensor measurement overflowed.
    MAS_TIMEOUT = 3, ///< The latest network measurement is too old.
    MAS_WAITING_FOR_FIRST_READ = 4, ///< No measurement has yet been received.
    MAS_UNKNOWN = 5 ///< The sensor state cannot be determined.
};

/**
 * @brief Device interface for one or more three-axis gyroscopes.
 *
 * Each measurement is a three-dimensional angular velocity `[x, y, z]`, in
 * degrees per second. Timestamps are expressed in seconds.
 */
class IThreeAxisGyroscopes
{
public:
    virtual ~IThreeAxisGyroscopes();
    virtual std::size_t getNrOfThreeAxisGyroscopes() const = 0;
    virtual MAS_status getThreeAxisGyroscopeStatus(std::size_t sens_index) const = 0;
    virtual bool getThreeAxisGyroscopeName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool
    getThreeAxisGyroscopeFrameName(std::size_t sens_index, std::string& frameName) const = 0;
    virtual bool getThreeAxisGyroscopeMeasure(std::size_t sens_index,
                                              VectorProxy<double>::Ref out,
                                              double& timestamp) const = 0;
};

class IThreeAxisGyroscopesSimulation
{
public:
    virtual ~IThreeAxisGyroscopesSimulation();
    virtual bool setThreeAxisGyroscopeStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setThreeAxisGyroscopeMeasure(std::size_t sens_index,
                                              const VectorProxy<const double>::Ref measure,
                                              double timestamp) = 0;
};

/**
 * @brief Device interface for one or more three-axis linear accelerometers.
 *
 * Measurements are `[x, y, z]` linear accelerations in metres per second
 * squared. Timestamps are expressed in seconds.
 */
class IThreeAxisLinearAccelerometers
{
public:
    virtual ~IThreeAxisLinearAccelerometers();
    virtual std::size_t getNrOfThreeAxisLinearAccelerometers() const = 0;
    virtual MAS_status getThreeAxisLinearAccelerometerStatus(std::size_t sens_index) const = 0;
    virtual bool
    getThreeAxisLinearAccelerometerName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool getThreeAxisLinearAccelerometerFrameName(std::size_t sens_index,
                                                          std::string& frameName) const = 0;
    virtual bool getThreeAxisLinearAccelerometerMeasure(std::size_t sens_index,
                                                        VectorProxy<double>::Ref out,
                                                        double& timestamp) const = 0;
};

class IThreeAxisLinearAccelerometersSimulation
{
public:
    virtual ~IThreeAxisLinearAccelerometersSimulation();
    virtual bool
    setThreeAxisLinearAccelerometerStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool
    setThreeAxisLinearAccelerometerMeasure(std::size_t sens_index,
                                           const VectorProxy<const double>::Ref measure,
                                           double timestamp) = 0;
};

/**
 * @brief Device interface for one or more three-axis angular accelerometers.
 *
 * Measurements are `[x, y, z]` angular accelerations in degrees per second
 * squared. Timestamps are expressed in seconds.
 */
class IThreeAxisAngularAccelerometers
{
public:
    virtual ~IThreeAxisAngularAccelerometers();
    virtual std::size_t getNrOfThreeAxisAngularAccelerometers() const = 0;
    virtual MAS_status getThreeAxisAngularAccelerometerStatus(std::size_t sens_index) const = 0;
    virtual bool
    getThreeAxisAngularAccelerometerName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool getThreeAxisAngularAccelerometerFrameName(std::size_t sens_index,
                                                           std::string& frameName) const = 0;
    virtual bool getThreeAxisAngularAccelerometerMeasure(std::size_t sens_index,
                                                         VectorProxy<double>::Ref out,
                                                         double& timestamp) const = 0;
};

class IThreeAxisAngularAccelerometersSimulation
{
public:
    virtual ~IThreeAxisAngularAccelerometersSimulation();
    virtual bool
    setThreeAxisAngularAccelerometerStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool
    setThreeAxisAngularAccelerometerMeasure(std::size_t sens_index,
                                            const VectorProxy<const double>::Ref measure,
                                            double timestamp) = 0;
};

/**
 * @brief Device interface for one or more three-axis magnetometers.
 *
 * Measurements are `[x, y, z]` magnetic-field values in tesla. Timestamps are
 * expressed in seconds.
 */
class IThreeAxisMagnetometers
{
public:
    virtual ~IThreeAxisMagnetometers();
    virtual std::size_t getNrOfThreeAxisMagnetometers() const = 0;
    virtual MAS_status getThreeAxisMagnetometerStatus(std::size_t sens_index) const = 0;
    virtual bool getThreeAxisMagnetometerName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool
    getThreeAxisMagnetometerFrameName(std::size_t sens_index, std::string& frameName) const = 0;
    virtual bool getThreeAxisMagnetometerMeasure(std::size_t sens_index,
                                                 VectorProxy<double>::Ref out,
                                                 double& timestamp) const = 0;
};

class IThreeAxisMagnetometersSimulation
{
public:
    virtual ~IThreeAxisMagnetometersSimulation();
    virtual bool setThreeAxisMagnetometerStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setThreeAxisMagnetometerMeasure(std::size_t sens_index,
                                                 const VectorProxy<const double>::Ref measure,
                                                 double timestamp) = 0;
};

/**
 * @brief Device interface for one or more position sensors.
 *
 * A position measurement is `[x, y, z]`, expressed in metres, from a
 * sensor-specific laboratory or surface-fixed frame to the sensor frame.
 * Timestamps are expressed in seconds.
 */
class IPositionSensors
{
public:
    virtual ~IPositionSensors();
    virtual std::size_t getNrOfPositionSensors() const = 0;
    virtual MAS_status getPositionSensorStatus(std::size_t sens_index) const = 0;
    virtual bool getPositionSensorName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool
    getPositionSensorFrameName(std::size_t sens_index, std::string& frameName) const = 0;
    virtual bool getPositionSensorMeasure(std::size_t sens_index,
                                          VectorProxy<double>::Ref xyz,
                                          double& timestamp) const = 0;
};

class IPositionSensorsSimulation
{
public:
    virtual ~IPositionSensorsSimulation();
    virtual bool setPositionSensorStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setPositionSensorMeasure(std::size_t sens_index,
                                          const VectorProxy<const double>::Ref measure,
                                          double timestamp) = 0;
};

/**
 * @brief Device interface for one or more linear-velocity sensors.
 *
 * Measurements are `[x, y, z]` linear velocities in metres per second.
 * Timestamps are expressed in seconds.
 */
class ILinearVelocitySensors
{
public:
    virtual ~ILinearVelocitySensors();
    virtual std::size_t getNrOfLinearVelocitySensors() const = 0;
    virtual MAS_status getLinearVelocitySensorStatus(std::size_t sens_index) const = 0;
    virtual bool getLinearVelocitySensorName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool
    getLinearVelocitySensorFrameName(std::size_t sens_index, std::string& frameName) const = 0;
    virtual bool getLinearVelocitySensorMeasure(std::size_t sens_index,
                                                VectorProxy<double>::Ref xyz,
                                                double& timestamp) const = 0;
};

class ILinearVelocitySensorsSimulation
{
public:
    virtual ~ILinearVelocitySensorsSimulation();
    virtual bool setLinearVelocitySensorStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setLinearVelocitySensorMeasure(std::size_t sens_index,
                                                const VectorProxy<const double>::Ref measure,
                                                double timestamp) = 0;
};

/**
 * @brief Device interface for one or more orientation sensors.
 *
 * Measurements are roll, pitch, and yaw in degrees. Let @f$f@f$ be the
 * laboratory or surface-fixed frame and @f$s@f$ the sensor-fixed frame. The
 * returned vector `[r, p, y]` has ranges @f$r \in [-180, 180]@f$,
 * @f$p \in [-90, 90]@f$, and @f$y \in [-180, 180]@f$, and represents
 *
 * @f[
 * {}^f R_s = R_z(y) R_y(p) R_x(r).
 * @f]
 *
 * The elementary rotations are
 * @f[
 * R_z(a) = \begin{bmatrix}\cos a & -\sin a & 0\\ \sin a & \cos a & 0\\ 0 & 0 & 1\end{bmatrix},
 * \quad R_y(a) = \begin{bmatrix}\cos a & 0 & \sin a\\ 0 & 1 & 0\\ -\sin a & 0 & \cos
 * a\end{bmatrix},
 * \quad R_x(a) = \begin{bmatrix}1 & 0 & 0\\ 0 & \cos a & -\sin a\\ 0 & \sin a & \cos
 * a\end{bmatrix}.
 * @f]
 *
 * In other words, yaw is applied about the fixed Z axis, then pitch about Y,
 * then roll about X. All angles are converted from degrees to radians before
 * evaluating the rotation matrices. Timestamps are expressed in seconds.
 */
class IOrientationSensors
{
public:
    virtual ~IOrientationSensors();
    virtual std::size_t getNrOfOrientationSensors() const = 0;
    virtual MAS_status getOrientationSensorStatus(std::size_t sens_index) const = 0;
    virtual bool getOrientationSensorName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool
    getOrientationSensorFrameName(std::size_t sens_index, std::string& frameName) const = 0;
    virtual bool getOrientationSensorMeasureAsRollPitchYaw(std::size_t sens_index,
                                                           VectorProxy<double>::Ref rpy,
                                                           double& timestamp) const = 0;
};

class IOrientationSensorsSimulation
{
public:
    virtual ~IOrientationSensorsSimulation();
    virtual bool setOrientationSensorStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setOrientationSensorMeasureAsRollPitchYaw(std::size_t sens_index,
                                                           const VectorProxy<const double>::Ref rpy,
                                                           double timestamp) = 0;
};

/**
 * @brief Device interface for one or more temperature sensors.
 *
 * Temperature measurements are scalar values in degrees Celsius. The vector
 * overload returns the same scalar as a one-element vector. Timestamps are
 * expressed in seconds.
 */
class ITemperatureSensors
{
public:
    virtual ~ITemperatureSensors();
    virtual std::size_t getNrOfTemperatureSensors() const = 0;
    virtual MAS_status getTemperatureSensorStatus(std::size_t sens_index) const = 0;
    virtual bool getTemperatureSensorName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool
    getTemperatureSensorFrameName(std::size_t sens_index, std::string& frameName) const = 0;
    virtual bool
    getTemperatureSensorMeasure(std::size_t sens_index, double& out, double& timestamp) const = 0;
    virtual bool getTemperatureSensorMeasure(std::size_t sens_index,
                                             VectorProxy<double>::Ref out,
                                             double& timestamp) const = 0;
};

class ITemperatureSensorsSimulation
{
public:
    virtual ~ITemperatureSensorsSimulation();
    virtual bool setTemperatureSensorStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool
    setTemperatureSensorMeasure(std::size_t sens_index, double measure, double timestamp) = 0;
};

/**
 * @brief Device interface for one or more six-axis force/torque sensors.
 *
 * Measurements have six elements: the first three are forces in newtons and
 * the last three are torques in newton metres. Timestamps are expressed in
 * seconds.
 */
class ISixAxisForceTorqueSensors
{
public:
    virtual ~ISixAxisForceTorqueSensors();
    virtual std::size_t getNrOfSixAxisForceTorqueSensors() const = 0;
    virtual MAS_status getSixAxisForceTorqueSensorStatus(std::size_t sens_index) const = 0;
    virtual bool
    getSixAxisForceTorqueSensorName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool
    getSixAxisForceTorqueSensorFrameName(std::size_t sens_index, std::string& frameName) const = 0;
    virtual bool getSixAxisForceTorqueSensorMeasure(std::size_t sens_index,
                                                    VectorProxy<double>::Ref out,
                                                    double& timestamp) const = 0;
};

class ISixAxisForceTorqueSensorsSimulation
{
public:
    virtual ~ISixAxisForceTorqueSensorsSimulation();
    virtual bool setSixAxisForceTorqueSensorStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setSixAxisForceTorqueSensorMeasure(std::size_t sens_index,
                                                    const VectorProxy<const double>::Ref measure,
                                                    double timestamp) = 0;
};

/**
 * @brief Device interface for one or more contact load-cell arrays.
 *
 * Each array reports normal-force values in newtons. The number of elements
 * in a particular array is returned by getContactLoadCellArraySize().
 * Timestamps are expressed in seconds.
 */
class IContactLoadCellArrays
{
public:
    virtual ~IContactLoadCellArrays();
    virtual std::size_t getNrOfContactLoadCellArrays() const = 0;
    virtual MAS_status getContactLoadCellArrayStatus(std::size_t sens_index) const = 0;
    virtual bool getContactLoadCellArrayName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool getContactLoadCellArrayMeasure(std::size_t sens_index,
                                                VectorProxy<double>::Ref out,
                                                double& timestamp) const = 0;
    virtual std::size_t getContactLoadCellArraySize(std::size_t sens_index) const = 0;
};

class IContactLoadCellArraysSimulation
{
public:
    virtual ~IContactLoadCellArraysSimulation();
    virtual bool setContactLoadCellArrayStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setContactLoadCellArrayMeasure(std::size_t sens_index,
                                                const VectorProxy<const double>::Ref measure,
                                                double timestamp) = 0;
};

/**
 * @brief Device interface for one or more encoder arrays.
 *
 * The size of each array is returned by getEncoderArraySize(). Units are
 * device-specific, as encoder arrays are often used for underactuated or
 * otherwise non-actuated mechanisms. Timestamps are expressed in seconds.
 */
class IEncoderArrays
{
public:
    virtual ~IEncoderArrays();
    virtual std::size_t getNrOfEncoderArrays() const = 0;
    virtual MAS_status getEncoderArrayStatus(std::size_t sens_index) const = 0;
    virtual bool getEncoderArrayName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool getEncoderArrayMeasure(std::size_t sens_index,
                                        VectorProxy<double>::Ref out,
                                        double& timestamp) const = 0;
    virtual std::size_t getEncoderArraySize(std::size_t sens_index) const = 0;
};

class IEncoderArraysSimulation
{
public:
    virtual ~IEncoderArraysSimulation();
    virtual bool setEncoderArrayStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setEncoderArrayMeasure(std::size_t sens_index,
                                        const VectorProxy<const double>::Ref measure,
                                        double timestamp) = 0;
};

/**
 * @brief Device interface for one or more tactile-sensor patches.
 *
 * The size of each patch is returned by getSkinPatchSize(). Measurement units
 * are implementation-specific. Timestamps are expressed in seconds.
 */
class ISkinPatches
{
public:
    virtual ~ISkinPatches();
    virtual std::size_t getNrOfSkinPatches() const = 0;
    virtual MAS_status getSkinPatchStatus(std::size_t sens_index) const = 0;
    virtual bool getSkinPatchName(std::size_t sens_index, std::string& name) const = 0;
    virtual bool getSkinPatchMeasure(std::size_t sens_index,
                                     VectorProxy<double>::Ref out,
                                     double& timestamp) const = 0;
    virtual std::size_t getSkinPatchSize(std::size_t sens_index) const = 0;
};

class ISkinPatchesSimulation
{
public:
    virtual ~ISkinPatchesSimulation();
    virtual bool setSkinPatchStatus(std::size_t sens_index, MAS_status status) = 0;
    virtual bool setSkinPatchMeasure(std::size_t sens_index,
                                     const VectorProxy<const double>::Ref measure,
                                     double timestamp) = 0;
};

} // namespace dinrail

#endif // DINRAIL_MULTIPLE_ANALOG_SENSORS_INTERFACES_H
