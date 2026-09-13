// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/JoypadAdapters.h>

namespace dinrail
{
bool InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>::getAxisCount(
    unsigned int& count)
{
    return source().getAxisCount(count);
}
bool InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>::getButtonCount(
    unsigned int& count)
{
    return source().getButtonCount(count);
}
bool InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>::getHatCount(
    unsigned int& count)
{
    return source().getHatCount(count);
}
bool InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>::getAxis(
    unsigned int axis, double& value)
{
    return source().getAxis(axis, value);
}
bool InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>::getButton(
    unsigned int button, float& value)
{
    return source().getButton(button, value);
}
bool InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>::getHat(
    unsigned int hat, unsigned char& value)
{
    return source().getHat(hat, value);
}
bool InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>::reconnect()
{
    return false;
}
bool InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>::getLastEvent(
    dinrail::JoypadDeviceEvent& event)
{
    event = dinrail::JoypadDeviceEvent::NoEvent;
    return true;
}

bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getAxisCount(
    unsigned int& count)
{
    return source().getAxisCount(count);
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getButtonCount(
    unsigned int& count)
{
    return source().getButtonCount(count);
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getHatCount(
    unsigned int& count)
{
    return source().getHatCount(count);
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getAxis(
    unsigned int axis, double& value)
{
    return source().getAxis(axis, value);
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getButton(
    unsigned int button, float& value)
{
    return source().getButton(button, value);
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getHat(
    unsigned int hat, unsigned char& value)
{
    return source().getHat(hat, value);
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getTrackballCount(
    unsigned int&)
{
    return false;
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getTouchSurfaceCount(
    unsigned int&)
{
    return false;
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getStickCount(
    unsigned int&)
{
    return false;
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getStickDoF(
    unsigned int, unsigned int&)
{
    return false;
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getTrackball(
    unsigned int, yarp::sig::Vector&)
{
    return false;
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getStick(
    unsigned int, yarp::sig::Vector&, yarp::dev::IJoypadController::JoypadCtrl_coordinateMode)
{
    return false;
}
bool InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>::getTouch(
    unsigned int, yarp::sig::Vector&)
{
    return false;
}

} // namespace dinrail
