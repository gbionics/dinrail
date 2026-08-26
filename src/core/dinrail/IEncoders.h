// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IENCODERS_H
#define DINRAIL_IENCODERS_H

#include <dinrail/VectorProxy.h>

namespace dinrail
{

/**
 * @brief Read-only interface for joint encoder measurements.
 *
 * This interface combines the measurement methods traditionally exposed by
 * `yarp::dev::IEncoders` and `yarp::dev::IEncodersTimed`. Methods that change or reset the
 * encoder value are intentionally not part of this measurement interface.
 *
 * Encoder measurement units depend on the joint type returned by `IAxisInfo::getJointType()`:
 *
 * - revolute joints use degrees, degrees per second, and degrees per second squared for position,
 *   speed, and acceleration, respectively;
 * - prismatic joints use metres, metres per second, and metres per second squared for position,
 *   speed, and acceleration, respectively.
 */
class IEncoders
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IEncoders();

    /**
     * @brief Get the number of controlled axes.
     * @param ax Output pointer receiving the number of axes.
     * @return True on success, false otherwise.
     */
    virtual bool getAxes(int* ax) = 0;

    /**
     * @brief Read the position measured by one encoder.
     * @param j Encoder index.
     * @param value Output pointer receiving the measured position, in degrees for a revolute joint
     * or metres for a prismatic joint.
     * @return True on success, false otherwise.
     */
    virtual bool getEncoder(int j, double* value) = 0;

    /**
     * @brief Read the positions measured by all encoders.
     * @param values Output vector receiving one position per controlled axis, in degrees for
     * revolute joints or metres for prismatic joints.
     * @return True on success, false otherwise.
     */
    virtual bool getEncoders(VectorProxy<double>::Ref values) = 0;

    /**
     * @brief Read one encoder position and its timestamp.
     * @param j Encoder index.
     * @param value Output pointer receiving the measured position, in degrees for a revolute joint
     * or metres for a prismatic joint.
     * @param timestamp Output pointer receiving the timestamp in seconds.
     * @return True on success, false otherwise.
     */
    virtual bool getEncoderTimed(int j, double* value, double* timestamp) = 0;

    /**
     * @brief Read all encoder positions and their timestamps.
     * @param values Output vector receiving one position per controlled axis, in degrees for
     * revolute joints or metres for prismatic joints.
     * @param timestamps Output vector receiving one timestamp in seconds per controlled axis.
     * @return True on success, false otherwise.
     */
    virtual bool
    getEncodersTimed(VectorProxy<double>::Ref values, VectorProxy<double>::Ref timestamps) = 0;

    /**
     * @brief Read the instantaneous speed measured by one encoder.
     * @param j Encoder index.
     * @param speed Output pointer receiving the measured speed, in degrees per second for a
     * revolute joint or metres per second for a prismatic joint.
     * @return True on success, false otherwise.
     */
    virtual bool getEncoderSpeed(int j, double* speed) = 0;

    /**
     * @brief Read the instantaneous speeds measured by all encoders.
     * @param speeds Output vector receiving one speed per controlled axis, in degrees per second
     * for revolute joints or metres per second for prismatic joints.
     * @return True on success, false otherwise.
     */
    virtual bool getEncoderSpeeds(VectorProxy<double>::Ref speeds) = 0;

    /**
     * @brief Read the instantaneous acceleration measured by one encoder.
     * @param j Encoder index.
     * @param acceleration Output pointer receiving the measured acceleration, in degrees per
     * second squared for a revolute joint or metres per second squared for a prismatic joint.
     * @return True on success, false otherwise.
     */
    virtual bool getEncoderAcceleration(int j, double* acceleration) = 0;

    /**
     * @brief Read the instantaneous accelerations measured by all encoders.
     * @param accelerations Output vector receiving one acceleration per controlled axis, in
     * degrees per second squared for revolute joints or metres per second squared for prismatic
     * joints.
     * @return True on success, false otherwise.
     */
    virtual bool getEncoderAccelerations(VectorProxy<double>::Ref accelerations) = 0;
};

/**
 * @brief Simulation interface used to provide joint encoder measurements.
 *
 * This interface is intended for simulators driving a fake device that implements `IEncoders`.
 * Its setters update simulated measurements; they do not represent encoder calibration or
 * hardware zeroing operations. Positions, speeds, and accelerations use the same joint-type-based
 * units documented by `IEncoders`.
 */
class IEncodersSimulation
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IEncodersSimulation();

    /**
     * @brief Set one simulated encoder position.
     * @param j Encoder index.
     * @param value Simulated position, in degrees for a revolute joint or metres for a prismatic
     * joint.
     * @return True on success, false otherwise.
     */
    virtual bool setEncoder(int j, double value) = 0;

    /**
     * @brief Set all simulated encoder positions.
     * @param values One position per controlled axis, in degrees for revolute joints or metres for
     * prismatic joints.
     * @return True on success, false otherwise.
     */
    virtual bool setEncoders(const VectorProxy<const double>::Ref values) = 0;

    /**
     * @brief Set one simulated encoder position and its timestamp.
     * @param j Encoder index.
     * @param value Simulated position, in degrees for a revolute joint or metres for a prismatic
     * joint.
     * @param timestamp Simulated timestamp in seconds.
     * @return True on success, false otherwise.
     */
    virtual bool setEncoderTimed(int j, double value, double timestamp) = 0;

    /**
     * @brief Set all simulated encoder positions and timestamps.
     * @param values One position per controlled axis, in degrees for revolute joints or metres for
     * prismatic joints.
     * @param timestamps One timestamp in seconds per controlled axis.
     * @return True on success, false otherwise.
     */
    virtual bool setEncodersTimed(const VectorProxy<const double>::Ref values,
                                  const VectorProxy<const double>::Ref timestamps) = 0;

    /**
     * @brief Set one simulated encoder speed.
     * @param j Encoder index.
     * @param speed Simulated speed, in degrees per second for a revolute joint or metres per second
     * for a prismatic joint.
     * @return True on success, false otherwise.
     */
    virtual bool setEncoderSpeed(int j, double speed) = 0;

    /**
     * @brief Set all simulated encoder speeds.
     * @param speeds One speed per controlled axis, in degrees per second for revolute joints or
     * metres per second for prismatic joints.
     * @return True on success, false otherwise.
     */
    virtual bool setEncoderSpeeds(const VectorProxy<const double>::Ref speeds) = 0;

    /**
     * @brief Set one simulated encoder acceleration.
     * @param j Encoder index.
     * @param acceleration Simulated acceleration, in degrees per second squared for a revolute
     * joint or metres per second squared for a prismatic joint.
     * @return True on success, false otherwise.
     */
    virtual bool setEncoderAcceleration(int j, double acceleration) = 0;

    /**
     * @brief Set all simulated encoder accelerations.
     * @param accelerations One acceleration per controlled axis, in degrees per second squared for
     * revolute joints or metres per second squared for prismatic joints.
     * @return True on success, false otherwise.
     */
    virtual bool setEncoderAccelerations(const VectorProxy<const double>::Ref accelerations) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IENCODERS_H
