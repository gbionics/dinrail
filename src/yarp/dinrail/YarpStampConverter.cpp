// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpStampConverter.h>

#include <yarp/os/Stamp.h>

#include <chrono>

namespace dinrail
{

yarp::os::Stamp toYarpStamp(const Stamp& stamp)
{
    const std::chrono::duration<double> timeInSeconds{stamp.time};
    return yarp::os::Stamp(static_cast<int>(stamp.sequenceNumber), timeInSeconds.count());
}

Stamp toDinrailStamp(const yarp::os::Stamp& stamp)
{
    const std::chrono::duration<double> timeInSeconds{stamp.getTime()};
    return Stamp{std::chrono::duration_cast<std::chrono::nanoseconds>(timeInSeconds),
                 static_cast<std::int32_t>(stamp.getCount())};
}

} // namespace dinrail
