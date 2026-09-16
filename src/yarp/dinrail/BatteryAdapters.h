// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DINRAILBATTERYINTERFACEADAPTERS_H
#define DINRAIL_DINRAILBATTERYINTERFACEADAPTERS_H

#include <dinrail/IBattery.h>
#include <dinrail/InterfaceAdapter.h>
#include <yarp/dev/IBattery.h>

namespace dinrail
{
template <>
class InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>
    : public InterfaceAdapterBase<yarp::dev::IBattery, dinrail::IBattery>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    yarp::dev::ReturnValue getBatteryVoltage(double& value) override;
    yarp::dev::ReturnValue getBatteryCurrent(double& value) override;
    yarp::dev::ReturnValue getBatteryCharge(double& value) override;
    yarp::dev::ReturnValue getBatteryTemperature(double& value) override;
    yarp::dev::ReturnValue getBatteryInfo(std::string& value) override;
    yarp::dev::ReturnValue getBatteryStatus(yarp::dev::IBattery::Battery_status& status) override;
};

template <>
class InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>
    : public InterfaceAdapterBase<dinrail::IBattery, yarp::dev::IBattery>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    Status getBatteryVoltage(double& voltage) override;
    Status getBatteryCurrent(double& current) override;
    Status getBatteryCharge(double& charge) override;
    Status getBatteryStatus(BatteryStatus& status) override;
    Status getBatteryTemperature(double& temperature) override;
    Status getBatteryInfo(std::string& batteryInfo) override;
};

} // namespace dinrail

#endif
