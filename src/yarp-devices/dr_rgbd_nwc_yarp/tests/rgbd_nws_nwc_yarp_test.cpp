// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/IRGBDSensor.h>

#include <yarp/dev/IRGBDSensor.h>
#include <yarp/dev/IWrapper.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Network.h>
#include <yarp/os/Property.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Image.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <functional>
#include <set>
#include <string>

namespace
{

constexpr int width = 32;
constexpr int height = 24;

bool waitFor(const std::function<bool()>& condition, double timeout = 3.0)
{
    const double deadline = yarp::os::Time::now() + timeout;
    while (yarp::os::Time::now() < deadline)
    {
        if (condition())
        {
            return true;
        }
        yarp::os::Time::delay(0.005);
    }
    return false;
}

yarp::os::Property clientConfig(const std::string& server, const std::string& client, bool withRgb)
{
    yarp::os::Property config;
    config.put("device", "dr_rgbd_nwc_yarp");
    config.put("remote_depth_port", server + "/depthImage:o");
    config.put("local_depth_port", client + "/depthImage:i");
    config.put("remote_rpc_port", server + "/rpc:i");
    config.put("local_rpc_port", client + "/rpc:o");
    if (withRgb)
    {
        config.put("remote_image_port", server + "/rgbImage:o");
        config.put("local_image_port", client + "/rgbImage:i");
    }
    return config;
}

} // namespace

TEST_CASE("dr_rgbd_nws_yarp and dr_rgbd_nwc_yarp stream a dinrail RGBD sensor", "[rgbd][yarp]")
{
    yarp::os::Network::setLocalMode(true);

    yarp::os::Property serverConfig;
    serverConfig.put("device", "dr_rgbd_nws_yarp");
    serverConfig.put("name", "/rgbdFake");
    serverConfig.put("subdevice", "dr_rgbd_fake");
    serverConfig.put("width", width);
    serverConfig.put("height", height);
    serverConfig.put("depth", 2.5);
    serverConfig.put("framerate", 30.0);
    yarp::dev::PolyDriver server(serverConfig);
    REQUIRE(server.isValid());

    SECTION("dr_rgbd_nwc_yarp receives the images and the parameters")
    {
        yarp::os::Property config = clientConfig("/rgbdFake", "/nwc", true);
        yarp::dev::PolyDriver client(config);
        REQUIRE(client.isValid());
        dinrail::IRGBDSensor* sensor = nullptr;
        REQUIRE(client.view(sensor));
        REQUIRE(sensor != nullptr);

        dinrail::CameraIntrinsics intrinsics;
        REQUIRE(sensor->getDepthIntrinsics(intrinsics).ok());
        CHECK(intrinsics.width == width);
        CHECK(intrinsics.height == height);
        CHECK(intrinsics.focalLengthX == Catch::Approx(500.0));
        CHECK(intrinsics.principalPointX == Catch::Approx(width / 2.0));
        dinrail::CameraExtrinsics extrinsics;
        REQUIRE(sensor->getDepthToRgbExtrinsics(extrinsics).ok());
        CHECK(extrinsics.rotation[0] == 1.0);
        CHECK(extrinsics.translation[0] == 0.0);

        dinrail::Image rgb;
        dinrail::DepthImage depth;
        REQUIRE(waitFor([&] { return sensor->getImages(rgb, depth).ok(); }));
        CHECK(depth.width == width);
        CHECK(depth.height == height);
        CHECK(depth.data == std::vector<float>(width * height, 2.5F));
        CHECK(rgb.format == dinrail::PixelFormat::Rgb8);
        CHECK(rgb.data == std::vector<std::uint8_t>(width * height * 3, 128));
        CHECK(rgb.stamp.sequenceNumber == depth.stamp.sequenceNumber);
        dinrail::Stamp stamp;
        REQUIRE(sensor->getImagesStamp(stamp).ok());
        CHECK(stamp.sequenceNumber >= depth.stamp.sequenceNumber);
        CHECK(sensor->getSensorStatus() == dinrail::RGBDSensorStatus::Ok);

        // The getters return the same images until new ones are received.
        dinrail::DepthImage again;
        REQUIRE(sensor->getDepthImage(again).ok());
        CHECK(again.stamp.sequenceNumber == depth.stamp.sequenceNumber);
    }

    SECTION("The color stream is optional")
    {
        yarp::os::Property config = clientConfig("/rgbdFake", "/nwcDepth", false);
        yarp::dev::PolyDriver client(config);
        REQUIRE(client.isValid());
        dinrail::IRGBDSensor* sensor = nullptr;
        REQUIRE(client.view(sensor));
        dinrail::DepthImage depth;
        REQUIRE(waitFor([&] { return sensor->getDepthImage(depth).ok(); }));
        CHECK(depth.data.front() == 2.5F);
        dinrail::Image rgb;
        CHECK(sensor->getImages(rgb, depth).code() == dinrail::StatusCode::FailedPrecondition);
    }

    SECTION("The YARP RGBDSensorClient is compatible with dr_rgbd_nws_yarp")
    {
        yarp::os::Property config;
        config.put("device", "RGBDSensorClient");
        config.put("localImagePort", "/yarpClient/rgbImage:i");
        config.put("localDepthPort", "/yarpClient/depthImage:i");
        config.put("localRpcPort", "/yarpClient/rpc:o");
        config.put("remoteImagePort", "/rgbdFake/rgbImage:o");
        config.put("remoteDepthPort", "/rgbdFake/depthImage:o");
        config.put("remoteRpcPort", "/rgbdFake/rpc:i");
        yarp::dev::PolyDriver client(config);
        REQUIRE(client.isValid());
        yarp::dev::IRGBDSensor* sensor = nullptr;
        REQUIRE(client.view(sensor));

        CHECK(sensor->getDepthWidth() == width);
        CHECK(sensor->getRgbHeight() == height);
        yarp::os::Property intrinsics;
        REQUIRE(sensor->getDepthIntrinsicParam(intrinsics));
        CHECK(intrinsics.find("focalLengthX").asFloat64() == Catch::Approx(500.0));
        CHECK(sensor->getSensorStatus() == yarp::dev::IRGBDSensor::RGBD_SENSOR_OK_IN_USE);
        yarp::sig::Matrix extrinsics;
        CHECK(sensor->getExtrinsicParam(extrinsics));

        yarp::sig::ImageOf<yarp::sig::PixelFloat> depth;
        REQUIRE(waitFor([&] { return sensor->getDepthImage(depth) && depth.width() > 0; }));
        CHECK(depth.pixel(3, 2) == 2.5F);
    }

    REQUIRE(server.close());
}

TEST_CASE("dr_rgbd_nws_yarp publishes new frames of an attached YARP sensor", "[rgbd][yarp]")
{
    yarp::os::Network::setLocalMode(true);

    yarp::os::Property cameraConfig;
    cameraConfig.put("device", "fakeDepthCamera");
    yarp::dev::PolyDriver camera(cameraConfig);
    REQUIRE(camera.isValid());

    yarp::os::Property serverConfig;
    serverConfig.put("device", "dr_rgbd_nws_yarp");
    serverConfig.put("name", "/yarpCamera");
    yarp::dev::PolyDriver server(serverConfig);
    REQUIRE(server.isValid());
    yarp::dev::IWrapper* wrapper = nullptr;
    REQUIRE(server.view(wrapper));
    REQUIRE(wrapper->attach(&camera));

    yarp::os::Property config = clientConfig("/yarpCamera", "/nwcYarpCamera", true);
    yarp::dev::PolyDriver client(config);
    REQUIRE(client.isValid());
    dinrail::IRGBDSensor* sensor = nullptr;
    REQUIRE(client.view(sensor));

    std::set<std::int32_t> sequenceNumbers;
    REQUIRE(waitFor([&] {
        dinrail::DepthImage depth;
        if (sensor->getDepthImage(depth).ok())
        {
            sequenceNumbers.insert(depth.stamp.sequenceNumber);
        }
        return sequenceNumbers.size() >= 3;
    }));

    REQUIRE(wrapper->detach());
    REQUIRE(client.close());
    REQUIRE(server.close());
    REQUIRE(camera.close());
}

TEST_CASE("dr_rgbd_nws_yarp publishes only the depth of a sensor without color", "[rgbd][yarp]")
{
    yarp::os::Network::setLocalMode(true);

    yarp::os::Property serverConfig;
    serverConfig.put("device", "dr_rgbd_nws_yarp");
    serverConfig.put("name", "/depthOnlyFake");
    serverConfig.put("subdevice", "dr_rgbd_fake");
    serverConfig.put("width", width);
    serverConfig.put("height", height);
    serverConfig.put("framerate", 30.0);
    serverConfig.put("enable_rgb", false);
    yarp::dev::PolyDriver server(serverConfig);
    REQUIRE(server.isValid());

    yarp::os::Property config = clientConfig("/depthOnlyFake", "/nwcDepthOnly", false);
    yarp::dev::PolyDriver client(config);
    REQUIRE(client.isValid());
    dinrail::IRGBDSensor* sensor = nullptr;
    REQUIRE(client.view(sensor));

    std::set<std::int32_t> sequenceNumbers;
    REQUIRE(waitFor([&] {
        dinrail::DepthImage depth;
        if (sensor->getDepthImage(depth).ok())
        {
            sequenceNumbers.insert(depth.stamp.sequenceNumber);
        }
        return sequenceNumbers.size() >= 3;
    }));
    dinrail::Stamp stamp;
    REQUIRE(sensor->getImagesStamp(stamp).ok());
    CHECK(stamp.sequenceNumber >= *sequenceNumbers.rbegin());

    REQUIRE(client.close());
    REQUIRE(server.close());
}

TEST_CASE("dr_rgbd_nws_yarp attached to dr_device_yarp uses the wrapped dinrail sensor",
          "[rgbd][yarp]")
{
    yarp::os::Network::setLocalMode(true);

    yarp::os::Property deviceConfig;
    deviceConfig.put("device", "dr_device_yarp");
    deviceConfig.put("subdevice", "dr_rgbd_fake");
    deviceConfig.put("width", width);
    deviceConfig.put("height", height);
    deviceConfig.put("framerate", 30.0);
    yarp::dev::PolyDriver device(deviceConfig);
    REQUIRE(device.isValid());
    // dr_device_yarp does not implement yarp::dev::IRGBDSensor, so the wrapper can only use the
    // dinrail sensor it wraps.
    yarp::dev::IRGBDSensor* yarpSensor = nullptr;
    CHECK_FALSE(device.view(yarpSensor));

    yarp::os::Property serverConfig;
    serverConfig.put("device", "dr_rgbd_nws_yarp");
    serverConfig.put("name", "/wrappedFake");
    yarp::dev::PolyDriver server(serverConfig);
    REQUIRE(server.isValid());
    yarp::dev::IWrapper* wrapper = nullptr;
    REQUIRE(server.view(wrapper));
    REQUIRE(wrapper->attach(&device));

    yarp::os::Property config = clientConfig("/wrappedFake", "/nwcWrappedFake", true);
    yarp::dev::PolyDriver client(config);
    REQUIRE(client.isValid());
    dinrail::IRGBDSensor* sensor = nullptr;
    REQUIRE(client.view(sensor));

    std::set<std::int32_t> sequenceNumbers;
    REQUIRE(waitFor([&] {
        dinrail::Image rgb;
        dinrail::DepthImage depth;
        if (sensor->getImages(rgb, depth).ok())
        {
            sequenceNumbers.insert(depth.stamp.sequenceNumber);
        }
        return sequenceNumbers.size() >= 3;
    }));

    REQUIRE(client.close());
    REQUIRE(wrapper->detach());
    REQUIRE(server.close());
    REQUIRE(device.close());
}
