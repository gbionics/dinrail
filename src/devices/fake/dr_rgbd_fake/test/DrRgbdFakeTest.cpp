// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Device.h>
#include <dinrail/IRGBDSensor.h>
#include <dinrail/Parameters.h>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("dr_rgbd_fake is driven through IRGBDSensorSimulation", "[device][rgbd]")
{
    dinrail::Parameters options;
    options.put("device", "dr_rgbd_fake");
    options.put("width", 4);
    options.put("height", 2);
    options.put("depth", 2.5);

    dinrail::Device device;
    REQUIRE(device.open(options));
    dinrail::IRGBDSensor* sensor = nullptr;
    dinrail::IRGBDSensorSimulation* simulation = nullptr;
    REQUIRE(device.view(sensor));
    REQUIRE(device.view(simulation));

    CHECK(sensor->getSensorStatus() == dinrail::RGBDSensorStatus::Ok);
    dinrail::CameraIntrinsics intrinsics;
    REQUIRE(sensor->getDepthIntrinsics(intrinsics).ok());
    CHECK(intrinsics.width == 4);
    CHECK(intrinsics.principalPointX == 2.0);

    dinrail::Image rgb;
    dinrail::DepthImage depth;
    REQUIRE(sensor->getImages(rgb, depth).ok());
    CHECK(rgb.format == dinrail::PixelFormat::Rgb8);
    CHECK(rgb.data.size() == 4 * 2 * 3);
    CHECK(depth.data == std::vector<float>(8, 2.5F));
    CHECK(depth.stamp.sequenceNumber == 0);

    depth.data[3] = 1.0F;
    depth.stamp.sequenceNumber = 7;
    rgb.stamp.sequenceNumber = 7;
    REQUIRE(simulation->setImages(rgb, depth).ok());
    dinrail::DepthImage updated;
    REQUIRE(sensor->getDepthImage(updated).ok());
    CHECK(updated.data[3] == 1.0F);
    CHECK(updated.stamp.sequenceNumber == 7);

    depth.width = 3;
    CHECK(simulation->setImages(rgb, depth).code() == dinrail::StatusCode::InvalidArgument);

    REQUIRE(simulation->setSensorStatus(dinrail::RGBDSensorStatus::Timeout).ok());
    CHECK(sensor->getSensorStatus() == dinrail::RGBDSensorStatus::Timeout);
    REQUIRE(device.close());
}
