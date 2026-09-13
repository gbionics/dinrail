// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DRMULTIPLEANALOGSENSORSFAKEYARP_H
#define DINRAIL_DRMULTIPLEANALOGSENSORSFAKEYARP_H

#include <dinrail/MultipleAnalogSensorsAdapters.h>
#include <dinrail/YarpDevice.h>

using DrMultipleAnalogSensorsFakeYarp = dinrail::YarpDevice<
    dinrail::InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes, dinrail::IThreeAxisGyroscopes>,
    dinrail::InterfaceAdapter<yarp::dev::IThreeAxisLinearAccelerometers,
                              dinrail::IThreeAxisLinearAccelerometers>,
    dinrail::InterfaceAdapter<yarp::dev::IThreeAxisAngularAccelerometers,
                              dinrail::IThreeAxisAngularAccelerometers>,
    dinrail::InterfaceAdapter<yarp::dev::IThreeAxisMagnetometers, dinrail::IThreeAxisMagnetometers>,
    dinrail::InterfaceAdapter<yarp::dev::IPositionSensors, dinrail::IPositionSensors>,
    dinrail::InterfaceAdapter<yarp::dev::ILinearVelocitySensors, dinrail::ILinearVelocitySensors>,
    dinrail::InterfaceAdapter<yarp::dev::IOrientationSensors, dinrail::IOrientationSensors>,
    dinrail::InterfaceAdapter<yarp::dev::ITemperatureSensors, dinrail::ITemperatureSensors>,
    dinrail::InterfaceAdapter<yarp::dev::ISixAxisForceTorqueSensors,
                              dinrail::ISixAxisForceTorqueSensors>,
    dinrail::InterfaceAdapter<yarp::dev::IContactLoadCellArrays, dinrail::IContactLoadCellArrays>,
    dinrail::InterfaceAdapter<yarp::dev::IEncoderArrays, dinrail::IEncoderArrays>,
    dinrail::InterfaceAdapter<yarp::dev::ISkinPatches, dinrail::ISkinPatches>>;

#endif
