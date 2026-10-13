// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause
#ifndef DINRAIL_DRCAMERAFAKEYARP_H
#define DINRAIL_DRCAMERAFAKEYARP_H
#include <dinrail/YarpCameraAdapters.h>
#include <dinrail/YarpDeviceFromDinrail.h>
#include <dinrail/YarpPreciselyTimedAdapters.h>
using DrCameraFakeYarp = dinrail::YarpDeviceFromDinrail<
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>,
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>,
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberImage, dinrail::IRGBDSensor>,
    dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>,
    dinrail::InterfaceAdapter<yarp::dev::IPreciselyTimed, dinrail::IPreciselyTimed>>;
#endif
