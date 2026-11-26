// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/IRGBDSensor.h>
#include <dinrail/IWrappedDinrailDevice.h>
#include <dinrail/MultipleAnalogSensorsInterfaces.h>

#include <yarp/dev/IWrapper.h>
#include <yarp/dev/MultipleAnalogSensorsInterfaces.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Network.h>
#include <yarp/os/Property.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("dr_device_yarp exposes the inertial sensors of the wrapped dinrail device",
          "[yarp][dr_device_yarp]")
{
    yarp::os::Network::setLocalMode(true);

    yarp::os::Property config;
    config.put("device", "dr_device_yarp");
    config.put("subdevice", "dr_multiplenalogsensors_fake");
    config.put("number_of_sensors", 2);
    yarp::dev::PolyDriver device(config);
    REQUIRE(device.isValid());

    dinrail::IWrappedDinrailDevice* wrapped = nullptr;
    REQUIRE(device.view(wrapped));
    dinrail::IThreeAxisGyroscopes* nativeGyroscopes = nullptr;
    REQUIRE(wrapped->wrappedDinrailDevice().view(nativeGyroscopes));

    yarp::dev::IThreeAxisGyroscopes* gyroscopes = nullptr;
    yarp::dev::IOrientationSensors* orientations = nullptr;
    REQUIRE(device.view(gyroscopes));
    REQUIRE(device.view(orientations));
    CHECK(gyroscopes->getNrOfThreeAxisGyroscopes() == 2);
    CHECK(orientations->getNrOfOrientationSensors() == 2);

    SECTION("multipleanalogsensorsserver publishes them")
    {
        yarp::os::Property serverConfig;
        serverConfig.put("device", "multipleanalogsensorsserver");
        serverConfig.put("name", "/wrappedImu");
        serverConfig.put("period", 10);
        yarp::dev::PolyDriver server(serverConfig);
        REQUIRE(server.isValid());
        yarp::dev::IWrapper* wrapper = nullptr;
        REQUIRE(server.view(wrapper));
        REQUIRE(wrapper->attach(&device));

        yarp::os::Property clientConfig;
        clientConfig.put("device", "multipleanalogsensorsclient");
        clientConfig.put("remote", "/wrappedImu");
        clientConfig.put("local", "/wrappedImuClient");
        yarp::dev::PolyDriver client(clientConfig);
        REQUIRE(client.isValid());
        yarp::dev::IThreeAxisGyroscopes* remoteGyroscopes = nullptr;
        REQUIRE(client.view(remoteGyroscopes));
        CHECK(remoteGyroscopes->getNrOfThreeAxisGyroscopes() == 2);

        yarp::sig::Vector measure;
        double timestamp = 0.0;
        bool received = false;
        const double deadline = yarp::os::Time::now() + 3.0;
        while (!received && yarp::os::Time::now() < deadline)
        {
            received = remoteGyroscopes->getThreeAxisGyroscopeMeasure(0, measure, timestamp);
            yarp::os::Time::delay(0.01);
        }
        CHECK(received);
        CHECK(measure.size() == 3);

        REQUIRE(client.close());
        REQUIRE(wrapper->detach());
        REQUIRE(server.close());
    }

    REQUIRE(device.close());
    CHECK(gyroscopes->getNrOfThreeAxisGyroscopes() == 0);
}

TEST_CASE("dr_device_yarp exposes no inertial sensors for a device without them",
          "[yarp][dr_device_yarp]")
{
    yarp::os::Property config;
    config.put("device", "dr_device_yarp");
    config.put("subdevice", "dr_rgbd_fake");
    yarp::dev::PolyDriver device(config);
    REQUIRE(device.isValid());

    yarp::dev::IThreeAxisGyroscopes* gyroscopes = nullptr;
    REQUIRE(device.view(gyroscopes));
    CHECK(gyroscopes->getNrOfThreeAxisGyroscopes() == 0);
    yarp::sig::Vector measure;
    double timestamp = 0.0;
    CHECK_FALSE(gyroscopes->getThreeAxisGyroscopeMeasure(0, measure, timestamp));

    dinrail::IWrappedDinrailDevice* wrapped = nullptr;
    REQUIRE(device.view(wrapped));
    dinrail::IRGBDSensor* sensor = nullptr;
    CHECK(wrapped->wrappedDinrailDevice().view(sensor));
}

TEST_CASE("dr_device_yarp requires the subdevice", "[yarp][dr_device_yarp]")
{
    yarp::os::Property config;
    config.put("device", "dr_device_yarp");
    yarp::dev::PolyDriver device(config);
    CHECK_FALSE(device.isValid());

    config.put("subdevice", "not_a_dinrail_device");
    CHECK_FALSE(device.open(config));
}
