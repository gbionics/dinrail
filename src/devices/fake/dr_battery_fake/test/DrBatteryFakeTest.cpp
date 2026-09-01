// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IBattery.h>
#include <dinrail/Parameters.h>

#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <string>

TEST_CASE("dr_battery_fake exposes battery interfaces", "[core][device][battery]")
{
    dinrail::Parameters options;
    options.put("device", "dr_battery_fake");

    dinrail::Device device;
    REQUIRE(device.open(options));

    dinrail::IBattery* battery = nullptr;
    REQUIRE(device.view(battery));
    REQUIRE(battery != nullptr);

    dinrail::IBatterySimulation* batterySimulation = nullptr;
    REQUIRE(device.view(batterySimulation));
    REQUIRE(batterySimulation != nullptr);

    REQUIRE(batterySimulation->setBatteryVoltage(48.0).ok());
    REQUIRE(batterySimulation->setBatteryCurrent(2.5).ok());
    REQUIRE(batterySimulation->setBatteryCharge(75.0).ok());
    REQUIRE(batterySimulation->setBatteryStatus(dinrail::BatteryStatus::OkInUse).ok());
    REQUIRE(batterySimulation->setBatteryTemperature(24.0).ok());
    REQUIRE(batterySimulation->setBatteryInfo("Test battery").ok());

    double voltage = 0.0;
    double current = 0.0;
    double charge = 0.0;
    double temperature = 0.0;
    dinrail::BatteryStatus status = dinrail::BatteryStatus::GeneralError;
    std::string info;

    REQUIRE(battery->getBatteryVoltage(voltage).ok());
    REQUIRE(battery->getBatteryCurrent(current).ok());
    REQUIRE(battery->getBatteryCharge(charge).ok());
    REQUIRE(battery->getBatteryStatus(status).ok());
    REQUIRE(battery->getBatteryTemperature(temperature).ok());
    REQUIRE(battery->getBatteryInfo(info).ok());

    REQUIRE(voltage == 48.0);
    REQUIRE(current == 2.5);
    REQUIRE(charge == 75.0);
    REQUIRE(status == dinrail::BatteryStatus::OkInUse);
    REQUIRE(temperature == 24.0);
    REQUIRE(info == "Test battery");

    REQUIRE(device.close());
}

TEST_CASE("dr_battery_fake validates charge", "[core][device][battery]")
{
    dinrail::Parameters options;
    options.put("device", "dr_battery_fake");

    dinrail::Device device;
    REQUIRE(device.open(options));

    dinrail::IBattery* battery = nullptr;
    REQUIRE(device.view(battery));

    dinrail::IBatterySimulation* batterySimulation = nullptr;
    REQUIRE(device.view(batterySimulation));

    REQUIRE(batterySimulation->setBatteryCharge(-1.0).code() == dinrail::StatusCode::OutOfRange);
    REQUIRE(batterySimulation->setBatteryCharge(101.0).code() == dinrail::StatusCode::OutOfRange);
    REQUIRE(batterySimulation->setBatteryCharge(std::numeric_limits<double>::quiet_NaN()).code()
            == dinrail::StatusCode::OutOfRange);

    double charge = 0.0;
    REQUIRE(battery->getBatteryCharge(charge).ok());
    REQUIRE(charge == 100.0);

    REQUIRE(device.close());
}

TEST_CASE("BatteryStatus values remain compatible with YARP IBattery", "[core][device][battery]")
{
    STATIC_REQUIRE(static_cast<int>(dinrail::BatteryStatus::OkStandby) == 0);
    STATIC_REQUIRE(static_cast<int>(dinrail::BatteryStatus::OkInCharge) == 1);
    STATIC_REQUIRE(static_cast<int>(dinrail::BatteryStatus::OkInUse) == 2);
    STATIC_REQUIRE(static_cast<int>(dinrail::BatteryStatus::GeneralError) == 3);
    STATIC_REQUIRE(static_cast<int>(dinrail::BatteryStatus::Timeout) == 4);
    STATIC_REQUIRE(static_cast<int>(dinrail::BatteryStatus::LowWarning) == 5);
    STATIC_REQUIRE(static_cast<int>(dinrail::BatteryStatus::CriticalWarning) == 6);
}
