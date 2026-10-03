// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpPreciselyTimedAdapters.h>

namespace dinrail
{
dinrail::Stamp
InterfaceAdapter<dinrail::IPreciselyTimed, yarp::dev::IPreciselyTimed>::getLastInputStamp()
{
    return toDinrailStamp(source().getLastInputStamp());
}

yarp::os::Stamp
InterfaceAdapter<yarp::dev::IPreciselyTimed, dinrail::IPreciselyTimed>::getLastInputStamp()
{
    return toYarpStamp(source().getLastInputStamp());
}
} // namespace dinrail
