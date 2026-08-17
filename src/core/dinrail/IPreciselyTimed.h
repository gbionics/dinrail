// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IPRECISELYTIMED_H
#define DINRAIL_IPRECISELYTIMED_H

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

} // namespace dinrail

#endif // DINRAIL_IPRECISELYTIMED_H
