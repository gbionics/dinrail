/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_CONTROLBOARD_YARP_PROTOCOL_SHARED_DEFINITIONS_H
#define DINRAIL_CONTROLBOARD_YARP_PROTOCOL_SHARED_DEFINITIONS_H

#include <yarp/os/Vocab.h>

namespace dinrail
{

// The base protocol version is 1000.0.0, to ensure that there
// may not be any confusion with nws/nwc protocol in YARP.
constexpr int CONTROLBOARD_YARP_PROTOCOL_VERSION_MAJOR = 1000;
constexpr int CONTROLBOARD_YARP_PROTOCOL_VERSION_MINOR = 0;
constexpr int CONTROLBOARD_YARP_PROTOCOL_VERSION_TWEAK = 0;

// Shared vocabulary used by Dinrail-specific RPC messages.
// Before adding more vocabs, always look in YARP source code to
// avoid conflicts with existing vocabs.
constexpr int VOCAB_DINRAIL_IMPEDANCE_ALL_SETPOINTS = yarp::os::createVocab32('d', 'i', 'a', 's');
constexpr int VOCAB_DINRAIL_SETPOINT = yarp::os::createVocab32('s', 'e', 't', 'p');

}

#endif // DINRAIL_CONTROLBOARD_YARP_PROTOCOL_SHARED_DEFINITIONS_H