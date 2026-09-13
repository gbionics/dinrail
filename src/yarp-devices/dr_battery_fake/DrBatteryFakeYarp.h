// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DRBATTERYFAKEYARP_H
#define DINRAIL_DRBATTERYFAKEYARP_H

#include <dinrail/BatteryAdapters.h>
#include <dinrail/YarpDevice.h>

using DrBatteryFakeYarp =
    dinrail::YarpDevice<dinrail::InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>>;

#endif
