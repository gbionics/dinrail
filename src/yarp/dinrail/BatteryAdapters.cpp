// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/BatteryAdapters.h>

#include <dinrail/IBattery.h>
#include <dinrail/YarpStatusConverter.h>

#include <yarp/dev/IBattery.h>

namespace dinrail
{

yarp::dev::ReturnValue
InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>::getBatteryVoltage(double& voltage)
{
    return toYarpReturnValue(source().getBatteryVoltage(voltage));
}

yarp::dev::ReturnValue
InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>::getBatteryCurrent(double& current)
{
    return toYarpReturnValue(source().getBatteryCurrent(current));
}

yarp::dev::ReturnValue
InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>::getBatteryCharge(double& charge)
{
    return toYarpReturnValue(source().getBatteryCharge(charge));
}

yarp::dev::ReturnValue
InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>::getBatteryTemperature(double& temperature)
{
    return toYarpReturnValue(source().getBatteryTemperature(temperature));
}

yarp::dev::ReturnValue
InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>::getBatteryInfo(std::string& batteryInfo)
{
    return toYarpReturnValue(source().getBatteryInfo(batteryInfo));
}

yarp::dev::ReturnValue InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>::getBatteryStatus(
    yarp::dev::IBattery::Battery_status& status)
{
    dinrail::BatteryStatus value{};
    auto result = source().getBatteryStatus(value);
    if (result.ok())
    {
        status = static_cast<yarp::dev::IBattery::Battery_status>(value);
    }
    return toYarpReturnValue(result);
}

dinrail::Status
InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>::getBatteryVoltage(double& voltage)
{
    return toDinrailStatus(source().getBatteryVoltage(voltage));
}

dinrail::Status
InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>::getBatteryCurrent(double& current)
{
    return toDinrailStatus(source().getBatteryCurrent(current));
}

dinrail::Status
InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>::getBatteryCharge(double& charge)
{
    return toDinrailStatus(source().getBatteryCharge(charge));
}

dinrail::Status InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>::getBatteryStatus(
    dinrail::BatteryStatus& status)
{
    yarp::dev::IBattery::Battery_status yarpStatus{};
    const dinrail::Status result = toDinrailStatus(source().getBatteryStatus(yarpStatus));
    if (result.ok())
    {
        status = static_cast<dinrail::BatteryStatus>(yarpStatus);
    }
    return result;
}

dinrail::Status
InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>::getBatteryTemperature(double& temperature)
{
    return toDinrailStatus(source().getBatteryTemperature(temperature));
}

dinrail::Status
InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>::getBatteryInfo(std::string& batteryInfo)
{
    return toDinrailStatus(source().getBatteryInfo(batteryInfo));
}
} // namespace dinrail
