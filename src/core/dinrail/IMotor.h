// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IMOTOR_H
#define DINRAIL_IMOTOR_H

#include <dinrail/VectorProxy.h>

namespace dinrail
{

/**
 * @brief Interface for motor information and motor-specific parameters.
 *
 * This interface mirrors the information-related part of `yarp::dev::IMotor`.
 * Temperatures are expressed in degrees Celsius. Gearbox ratios are
 * device-specific quantities and use the conventions of the underlying motor
 * interface.
 */
class IMotor
{
public:
    /** @brief Virtual destructor. */
    virtual ~IMotor();

    /**
     * @brief Get the number of available motors.
     * @param num Output pointer receiving the number of motors.
     * @return True on success, false otherwise.
     */
    virtual bool getNumberOfMotors(int* num) = 0;

    /**
     * @brief Get the temperature of one motor.
     * @param m Motor index.
     * @param value Output pointer receiving the temperature in degrees Celsius.
     * @return True on success, false otherwise.
     */
    virtual bool getTemperature(int m, double* value) = 0;

    /**
     * @brief Get the temperatures of all motors.
     * @param values Output vector receiving one temperature per motor.
     * @return True on success, false otherwise.
     */
    virtual bool getTemperatures(VectorProxy<double>::Ref values) = 0;

    /**
     * @brief Get the temperature limit of one motor.
     * @param m Motor index.
     * @param temperature Output pointer receiving the temperature limit in degrees Celsius.
     * @return True on success, false otherwise.
     */
    virtual bool getTemperatureLimit(int m, double* temperature) = 0;

    /**
     * @brief Set the temperature limit of one motor.
     * @param m Motor index.
     * @param temperature Temperature limit in degrees Celsius.
     * @return True on success, false otherwise.
     */
    virtual bool setTemperatureLimit(int m, double temperature) = 0;

    /**
     * @brief Get the gearbox ratio of one motor.
     * @param m Motor index.
     * @param value Output pointer receiving the gearbox ratio.
     * @return True on success, false otherwise.
     */
    virtual bool getGearboxRatio(int m, double* value) = 0;

    /**
     * @brief Set the gearbox ratio of one motor.
     * @param m Motor index.
     * @param value Gearbox ratio.
     * @return True on success, false otherwise.
     */
    virtual bool setGearboxRatio(int m, double value) = 0;
};

/**
 * @brief Simulation interface used to provide motor temperatures.
 *
 * The setters update simulated measurements. They do not model a physical
 * motor temperature controller.
 */
class IMotorSimulation
{
public:
    /** @brief Virtual destructor. */
    virtual ~IMotorSimulation();

    /**
     * @brief Set the simulated temperature of one motor.
     * @param m Motor index.
     * @param value Temperature in degrees Celsius.
     * @return True on success, false otherwise.
     */
    virtual bool setTemperature(int m, double value) = 0;

    /**
     * @brief Set the simulated temperatures of all motors.
     * @param values One temperature in degrees Celsius per motor.
     * @return True on success, false otherwise.
     */
    virtual bool setTemperatures(const VectorProxy<const double>::Ref values) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IMOTOR_H
