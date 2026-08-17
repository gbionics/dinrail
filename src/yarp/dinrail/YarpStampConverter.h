// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPSTAMPCONVERTER_H
#define DINRAIL_YARPSTAMPCONVERTER_H

#include <dinrail/IPreciselyTimed.h>

namespace yarp::os
{
class Stamp;
}

namespace dinrail
{

/**
 * @brief Convert a dinrail timestamp to its YARP representation.
 *
 * YARP stores time as seconds in a double, so a conversion to YARP may lose
 * nanosecond precision for timestamps with a large absolute value.
 */
yarp::os::Stamp toYarpStamp(const Stamp& stamp);

/**
 * @brief Convert a YARP timestamp to its dinrail representation.
 *
 * The YARP time in seconds is converted to whole nanoseconds.
 */
Stamp toDinrailStamp(const yarp::os::Stamp& stamp);

} // namespace dinrail

#endif // DINRAIL_YARPSTAMPCONVERTER_H
