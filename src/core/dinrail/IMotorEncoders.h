// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IMOTORENCODERS_H
#define DINRAIL_IMOTORENCODERS_H

#include <dinrail/VectorProxy.h>

namespace dinrail
{

/**
 * @brief Read-only interface for motor encoder measurements.
 *
 * This interface is inspired by `yarp::dev::IMotorEncoders`. Operations that
 * set or reset encoder values are intentionally not part of the measurement
 * interface. Motor encoder values, speeds, and accelerations use the same
 * units as the corresponding joint encoder interface.
 */
class IMotorEncoders
{
public:
    /** @brief Virtual destructor. */
    virtual ~IMotorEncoders();

    /**
     * @brief Get the number of available motor encoders.
     * @param num Output pointer receiving the number of motor encoders.
     * @return True on success, false otherwise.
     */
    virtual bool getNumberOfMotorEncoders(int* num) = 0;

    /**
     * @brief Get the counts per revolution of one motor encoder.
     * @param m Motor encoder index.
     * @param countsPerRevolution Output pointer receiving the counts per revolution.
     * @return True on success, false otherwise.
     */
    virtual bool getMotorEncoderCountsPerRevolution(int m, double* countsPerRevolution) = 0;

    /**
     * @brief Read one motor encoder position.
     * @param m Motor encoder index.
     * @param value Output pointer receiving the position.
     * @return True on success, false otherwise.
     */
    virtual bool getMotorEncoder(int m, double* value) = 0;

    /**
     * @brief Read all motor encoder positions.
     * @param values Output vector receiving one position per motor encoder.
     * @return True on success, false otherwise.
     */
    virtual bool getMotorEncoders(VectorProxy<double>::Ref values) = 0;

    /**
     * @brief Read one motor encoder position and timestamp.
     * @param m Motor encoder index.
     * @param value Output pointer receiving the position.
     * @param timestamp Output pointer receiving the timestamp in seconds.
     * @return True on success, false otherwise.
     */
    virtual bool getMotorEncoderTimed(int m, double* value, double* timestamp) = 0;

    /**
     * @brief Read all motor encoder positions and timestamps.
     * @param values Output vector receiving the positions.
     * @param timestamps Output vector receiving one timestamp in seconds per encoder.
     * @return True on success, false otherwise.
     */
    virtual bool
    getMotorEncodersTimed(VectorProxy<double>::Ref values, VectorProxy<double>::Ref timestamps) = 0;

    /**
     * @brief Read one motor encoder speed.
     * @param m Motor encoder index.
     * @param speed Output pointer receiving the speed.
     * @return True on success, false otherwise.
     */
    virtual bool getMotorEncoderSpeed(int m, double* speed) = 0;

    /**
     * @brief Read all motor encoder speeds.
     * @param speeds Output vector receiving one speed per motor encoder.
     * @return True on success, false otherwise.
     */
    virtual bool getMotorEncoderSpeeds(VectorProxy<double>::Ref speeds) = 0;

    /**
     * @brief Read one motor encoder acceleration.
     * @param m Motor encoder index.
     * @param acceleration Output pointer receiving the acceleration.
     * @return True on success, false otherwise.
     */
    virtual bool getMotorEncoderAcceleration(int m, double* acceleration) = 0;

    /**
     * @brief Read all motor encoder accelerations.
     * @param accelerations Output vector receiving one acceleration per motor encoder.
     * @return True on success, false otherwise.
     */
    virtual bool getMotorEncoderAccelerations(VectorProxy<double>::Ref accelerations) = 0;
};

/**
 * @brief Simulation interface used to provide motor encoder measurements.
 *
 * The setters update simulated measurements and do not represent encoder
 * calibration or hardware zeroing operations.
 */
class IMotorEncodersSimulation
{
public:
    /** @brief Virtual destructor. */
    virtual ~IMotorEncodersSimulation();

    /** @brief Set one simulated motor encoder position. */
    virtual bool setMotorEncoder(int m, double value) = 0;

    /** @brief Set all simulated motor encoder positions. */
    virtual bool setMotorEncoders(const VectorProxy<const double>::Ref values) = 0;

    /** @brief Set one simulated motor encoder position and timestamp. */
    virtual bool setMotorEncoderTimed(int m, double value, double timestamp) = 0;

    /** @brief Set all simulated motor encoder positions and timestamps. */
    virtual bool setMotorEncodersTimed(const VectorProxy<const double>::Ref values,
                                       const VectorProxy<const double>::Ref timestamps) = 0;

    /** @brief Set one simulated motor encoder speed. */
    virtual bool setMotorEncoderSpeed(int m, double speed) = 0;

    /** @brief Set all simulated motor encoder speeds. */
    virtual bool setMotorEncoderSpeeds(const VectorProxy<const double>::Ref speeds) = 0;

    /** @brief Set one simulated motor encoder acceleration. */
    virtual bool setMotorEncoderAcceleration(int m, double acceleration) = 0;

    /** @brief Set all simulated motor encoder accelerations. */
    virtual bool
    setMotorEncoderAccelerations(const VectorProxy<const double>::Ref accelerations) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IMOTORENCODERS_H
