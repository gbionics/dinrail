// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_JOYPADADAPTERS_H
#define DINRAIL_JOYPADADAPTERS_H

#include <dinrail/IJoypadControl.h>
#include <dinrail/InterfaceAdapter.h>

#include <yarp/dev/IJoypadController.h>

namespace dinrail
{

template <>
class InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IJoypadController, dinrail::IJoypadControl>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool getAxisCount(unsigned int& count) override;
    bool getButtonCount(unsigned int& count) override;
    bool getHatCount(unsigned int& count) override;
    bool getAxis(unsigned int axis, double& value) override;
    bool getButton(unsigned int button, float& value) override;
    bool getHat(unsigned int hat, unsigned char& value) override;
    bool getTrackballCount(unsigned int&) override;
    bool getTouchSurfaceCount(unsigned int&) override;
    bool getStickCount(unsigned int&) override;
    bool getStickDoF(unsigned int, unsigned int&) override;
    bool getTrackball(unsigned int, yarp::sig::Vector&) override;
    bool getStick(unsigned int,
                  yarp::sig::Vector&,
                  yarp::dev::IJoypadController::JoypadCtrl_coordinateMode) override;
    bool getTouch(unsigned int, yarp::sig::Vector&) override;
};

template <>
class InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>
    : public InterfaceAdapterBase<dinrail::IJoypadControl, yarp::dev::IJoypadController>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool getAxisCount(unsigned int& count) override;
    bool getButtonCount(unsigned int& count) override;
    bool getHatCount(unsigned int& count) override;
    bool getAxis(unsigned int axis, double& value) override;
    bool getButton(unsigned int button, float& value) override;
    bool getHat(unsigned int hat, unsigned char& value) override;
    bool reconnect() override;
    bool getLastEvent(JoypadDeviceEvent& event) override;
};

} // namespace dinrail

#endif // DINRAIL_JOYPADADAPTERS_H
