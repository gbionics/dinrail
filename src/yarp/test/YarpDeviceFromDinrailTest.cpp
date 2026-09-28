// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <DrBatteryFakeYarp.h>

#include <catch2/catch_test_macros.hpp>

#include <dinrail/IBattery.h>

#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/IBattery.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Property.h>

TEST_CASE("YarpDeviceFromDinrail exposes selected adapters")
{
    DrBatteryFakeYarp wrapper;
    yarp::os::Property config;
    config.put("device", "dr_battery_fake");
    REQUIRE(wrapper.open(config));

    dinrail::IBatterySimulation* simulation = nullptr;
    REQUIRE(wrapper.device().view(simulation));
    REQUIRE(simulation != nullptr);
    REQUIRE(simulation->setBatteryVoltage(48.0).ok());

    yarp::dev::IBattery* battery = nullptr;
    REQUIRE(wrapper.view(battery));
    REQUIRE(battery != nullptr);
    double voltage = 0.0;
    REQUIRE(battery->getBatteryVoltage(voltage));
    REQUIRE(voltage == 48.0);

    yarp::dev::IEncoders* unsupported = nullptr;
    REQUIRE_FALSE(wrapper.view(unsupported));

    REQUIRE(wrapper.close());
    REQUIRE_FALSE(wrapper.device().isValid());
}

TEST_CASE("YarpDeviceFromDinrail can reopen its native device")
{
    DrBatteryFakeYarp wrapper;
    yarp::os::Property config;
    config.put("device", "dr_battery_fake");
    REQUIRE(wrapper.open(config));
    REQUIRE(wrapper.close());
    REQUIRE(wrapper.open(config));

    yarp::dev::IBattery* battery = nullptr;
    REQUIRE(wrapper.view(battery));
    REQUIRE(battery != nullptr);
    REQUIRE(wrapper.close());
}

TEST_CASE("A YarpDeviceFromDinrail wrapper loads as an installed YARP plugin")
{
    yarp::os::Property config;
    config.put("device", "dr_battery_fake");
    yarp::dev::PolyDriver driver;
    REQUIRE(driver.open(config));

    yarp::dev::IBattery* battery = nullptr;
    REQUIRE(driver.view(battery));
    REQUIRE(battery != nullptr);

    yarp::dev::IEncoders* unsupported = nullptr;
    REQUIRE_FALSE(driver.view(unsupported));
    REQUIRE(driver.close());
}
