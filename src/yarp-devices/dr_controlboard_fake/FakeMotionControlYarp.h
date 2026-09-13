// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_FAKEMOTIONCONTROLYARP_H
#define DINRAIL_FAKEMOTIONCONTROLYARP_H

#include <dinrail/CommonAdapters.h>
#include <dinrail/ControlBoardAdapters.h>
#include <dinrail/YarpDevice.h>

using FakeMotionControlYarp = dinrail::YarpDevice<
    dinrail::InterfaceAdapter<yarp::dev::IAxisInfo, dinrail::IAxisInfo>,
    dinrail::InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>,
    dinrail::InterfaceAdapter<yarp::dev::IJointFault, dinrail::IJointFault>,
    dinrail::InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>,
    dinrail::InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>,
    dinrail::InterfaceAdapter<yarp::dev::IPreciselyTimed, dinrail::IPreciselyTimed>,
    dinrail::InterfaceAdapter<dinrail::IImpedanceAllSetPointsControl,
                              dinrail::IImpedanceAllSetPointsControl>>;

#endif
