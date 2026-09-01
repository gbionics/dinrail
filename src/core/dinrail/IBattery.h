// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IBATTERY_H
#define DINRAIL_IBATTERY_H

#include <dinrail/Status.h>

#include <string>

namespace dinrail
{

/**
 * @brief Operating and health status reported by a battery.
 */
enum class BatteryStatus : int
{
    /** The battery is healthy and in standby. */
    OkStandby = 0,
    /** The battery is healthy and charging. */
    OkInCharge = 1,
    /** The battery is healthy and in use. */
    OkInUse = 2,
    /** The battery reported a general error. */
    GeneralError = 3,
    /** Communication with the battery timed out. */
    Timeout = 4,
    /** The battery charge is low. */
    LowWarning = 5,
    /** The battery charge is critically low. */
    CriticalWarning = 6,
};

/**
 * @brief Read-only interface for battery measurements and status.
 *
 * This interface mirrors the measurements exposed by `yarp::dev::IBattery`,
 * using `dinrail::Status` to report the outcome of each operation.
 */
class IBattery
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IBattery();

    /**
     * @brief Get the instantaneous battery voltage.
     * @param voltage Output variable receiving the voltage measurement.
     * @return The outcome of the operation.
     */
    virtual Status getBatteryVoltage(double& voltage) = 0;

    /**
     * @brief Get the instantaneous battery current.
     * @param current Output variable receiving the current measurement.
     * @return The outcome of the operation.
     */
    virtual Status getBatteryCurrent(double& current) = 0;

    /**
     * @brief Get the battery state of charge.
     * @param charge Output variable receiving the charge percentage in the range [0, 100].
     * @return The outcome of the operation.
     */
    virtual Status getBatteryCharge(double& charge) = 0;

    /**
     * @brief Get the battery operating and health status.
     * @param status Output variable receiving the battery status.
     * @return The outcome of the operation.
     */
    virtual Status getBatteryStatus(BatteryStatus& status) = 0;

    /**
     * @brief Get the battery temperature.
     * @param temperature Output variable receiving the temperature measurement.
     * @return The outcome of the operation.
     */
    virtual Status getBatteryTemperature(double& temperature) = 0;

    /**
     * @brief Get battery hardware information, such as its maximum voltage.
     * @param batteryInfo Output string receiving the battery information.
     * @return The outcome of the operation.
     */
    virtual Status getBatteryInfo(std::string& batteryInfo) = 0;
};

/**
 * @brief Simulation interface used to provide battery measurements and status.
 *
 * This interface is intended for simulators driving a fake device that implements `IBattery`.
 * Its setters update simulated battery data; they do not control a physical charger or battery.
 */
class IBatterySimulation
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IBatterySimulation();

    /**
     * @brief Set the simulated instantaneous battery voltage.
     * @param voltage The voltage measurement to report.
     * @return The outcome of the operation.
     */
    virtual Status setBatteryVoltage(double voltage) = 0;

    /**
     * @brief Set the simulated instantaneous battery current.
     * @param current The current measurement to report.
     * @return The outcome of the operation.
     */
    virtual Status setBatteryCurrent(double current) = 0;

    /**
     * @brief Set the simulated battery state of charge.
     * @param charge The charge percentage to report, in the range [0, 100].
     * @return The outcome of the operation.
     */
    virtual Status setBatteryCharge(double charge) = 0;

    /**
     * @brief Set the simulated battery operating and health status.
     * @param status The battery status to report.
     * @return The outcome of the operation.
     */
    virtual Status setBatteryStatus(BatteryStatus status) = 0;

    /**
     * @brief Set the simulated battery temperature.
     * @param temperature The temperature measurement to report.
     * @return The outcome of the operation.
     */
    virtual Status setBatteryTemperature(double temperature) = 0;

    /**
     * @brief Set the simulated battery hardware information.
     * @param batteryInfo The battery information to report.
     * @return The outcome of the operation.
     */
    virtual Status setBatteryInfo(const std::string& batteryInfo) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IBATTERY_H
