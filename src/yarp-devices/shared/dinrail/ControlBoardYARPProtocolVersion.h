/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_CONTROLBOARD_YARP_PROTOCOLVERSION_H
#define DINRAIL_CONTROLBOARD_YARP_PROTOCOLVERSION_H

namespace dinrail
{

// The base protocol version is 1000.0.0, to ensure that there
// may not be any confusion with nws/nwc protocol in YARP
constexpr int CONTROLBOARD_YARP_PROTOCOL_VERSION_MAJOR = 1000;
constexpr int CONTROLBOARD_YARP_PROTOCOL_VERSION_MINOR = 0;
constexpr int CONTROLBOARD_YARP_PROTOCOL_VERSION_TWEAK = 0;

}

#endif // DINRAIL_CONTROLBOARD_YARP_PROTOCOLVERSION_H
