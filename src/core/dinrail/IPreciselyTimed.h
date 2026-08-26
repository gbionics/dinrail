// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IPRECISELYTIMED_H
#define DINRAIL_IPRECISELYTIMED_H

#include <chrono>
#include <cstdint>

namespace dinrail
{

/**
 * @brief Timestamp and sequence number associated with a data sample.
 *
 * The timestamp is represented as a duration in nanoseconds from an arbitrary
 * epoch.
 */
struct Stamp
{
    std::chrono::nanoseconds time{0};
    std::int32_t sequenceNumber{0};
};

/**
 * @brief Interface for devices that provide a timestamp for the last measurement.
 *
 * This interface is typically used for devices that provide some measurement or
 * acquisition, and the timestamp of the last measurement is used to synchronize the
 * data with other devices.
 */
class IPreciselyTimed
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IPreciselyTimed();

    /**
     * @brief Return the timestamp associated with the last measurement.
     * @return The timestamp and sequence number of the last measurement.
     */
    virtual Stamp getLastInputStamp() = 0;
};

/**
 * @brief Simulation interface for devices that provide a timestamp for the last measurement.
 *
 * This interface is typically used by `fake` devices that implement the `IPreciselyTimed`
 * interface, to provide a way for simulators to set the timestamp of the last measurement returned
 * by the `fake` device.
 */
class IPreciselyTimedSimulation
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IPreciselyTimedSimulation();

    /**
     * @brief Set the timestamp associated with the last measurement.
     * @param stamp The timestamp and sequence number of the last measurement.
     */
    virtual void setLastInputStamp(const Stamp& stamp) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IPRECISELYTIMED_H
