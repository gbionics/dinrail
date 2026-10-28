// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause
#include "../DrCameraFake.h"
#include <catch2/catch_test_macros.hpp>
#include <dinrail/Device.h>
#include <limits>

TEST_CASE("Camera plugin exposes acquisition and simulation")
{
    // Open the plugin with small RGB and depth frames.
    dinrail::Parameters config;
    config.put("device", "dr_camera_fake");
    config.put("width", 3);
    config.put("height", 2);
    config.put("depth_width", 3);
    config.put("depth_height", 2);
    dinrail::Device device;
    REQUIRE(device.open(config));

    // Obtain acquisition and simulator interfaces from the loaded plugin.
    dinrail::IRGBDSensor* camera = nullptr;
    dinrail::IRGBDSensorSimulation* simulation = nullptr;
    dinrail::IFrameGrabberImageRawSimulation* rawSimulation = nullptr;
    REQUIRE(device.view(camera));
    REQUIRE(device.view(simulation));
    REQUIRE(device.view(rawSimulation));

    // Supply a non-contiguous RGB region and a depth frame with independent stamps.
    cv::Mat backing(4, 5, CV_8UC3, cv::Scalar(10, 20, 30));
    cv::Mat rgb = backing(cv::Rect(1, 1, 3, 2));
    REQUIRE_FALSE(rgb.isContinuous());
    cv::Mat depth(2, 3, CV_32FC1, cv::Scalar(1.25));
    dinrail::Stamp rgbStamp{std::chrono::nanoseconds(123456789), 7};
    dinrail::Stamp depthStamp{std::chrono::nanoseconds(123456799), 8};
    REQUIRE(simulation->setImages(rgb, depth, rgbStamp, depthStamp).ok());

    // Changing the simulator buffers must not change the stored frames.
    rgb.setTo(cv::Scalar(0, 0, 0));
    depth.setTo(0);
    cv::Mat outRgb, outDepth;
    dinrail::Stamp rs, ds;
    REQUIRE(camera->getImages(outRgb, outDepth, &rs, &ds).ok());
    REQUIRE(outRgb.at<cv::Vec3b>(0, 0) == cv::Vec3b(10, 20, 30));
    REQUIRE(outDepth.at<float>(0, 0) == 1.25f);
    REQUIRE(rs.time == rgbStamp.time);
    REQUIRE(ds.sequenceNumber == 8);

    // Changing a returned frame must not change the next acquisition.
    outDepth.setTo(0);
    REQUIRE(camera->getDepthImage(outDepth).ok());
    REQUIRE(outDepth.at<float>(0, 0) == 1.25f);

    // Reject mismatched frame dimensions without replacing the stored data.
    REQUIRE_FALSE(simulation->setImages(outRgb, cv::Mat(1, 1, CV_32FC1), {}, {}).ok());
    REQUIRE(camera->getRgbImage(outRgb, &rs).ok());
    REQUIRE(rs.time == rgbStamp.time);

    // The raw simulation interface requires monochrome input.
    REQUIRE_FALSE(rawSimulation->setImage(outRgb).ok());

    // Release the loaded plugin.
    REQUIRE(device.close());
}

TEST_CASE("Camera modes, lifecycle and validation")
{
    dinrail::DrCameraFake camera;
    dinrail::Parameters config;
    cv::Mat image;
    REQUIRE(camera.getImage(image).code() == dinrail::StatusCode::FailedPrecondition);

    // Invalid modes and dimensions must fail during open.
    config.put("mode", "bad");
    REQUIRE_FALSE(camera.open(config));
    config.put("mode", "framegrabber");
    config.put("width", -1);
    REQUIRE_FALSE(camera.open(config));

    // Framegrabber mode provides monochrome frames and disables depth acquisition.
    config.put("width", 3);
    config.put("height", 2);
    REQUIRE(camera.open(config));
    REQUIRE(camera.getDepthImage(image).code() == dinrail::StatusCode::Unimplemented);
    REQUIRE(camera.getImage(image).ok());
    REQUIRE(image.type() == CV_8UC1);

    // Validate feature values and honor the simulated feature capabilities.
    REQUIRE_FALSE(camera.setFeature(0, std::numeric_limits<double>::quiet_NaN()).ok());
    REQUIRE(camera.setFeatureCapabilities(0, true, false, false, true, false).ok());
    REQUIRE(camera.setMode(0, dinrail::FeatureMode::Auto).code()
            == dinrail::StatusCode::Unimplemented);
    REQUIRE_FALSE(camera.setFeature(-1, 0.5).ok());

    // Both components of a feature value must round-trip.
    REQUIRE(camera.setFeature(0, 0.25, 0.75).ok());
    double a, b;
    REQUIRE(camera.getFeature(0, a, b).ok());
    REQUIRE(a == 0.25);
    REQUIRE(b == 0.75);

    // Closing the device clears its reported dimensions.
    REQUIRE(camera.close());
    REQUIRE(camera.getRgbWidth() == 0);

    // Reopen in RGBD mode and validate geometry settings.
    config.put("mode", "rgbd");
    REQUIRE(camera.open(config));
    REQUIRE_FALSE(camera.setDepthClipPlanes(2, 1).ok());
    REQUIRE_FALSE(camera.setDepthAccuracy(0).ok());
    REQUIRE_FALSE(camera.setRgbFOV(180, 45).ok());

    // A resolution change also updates the generated calibration.
    REQUIRE(camera.setRgbResolution(5, 4).ok());
    REQUIRE(camera.width() == 5);
    dinrail::Parameters intr;
    REQUIRE(camera.getRgbIntrinsicParam(intr).ok());
    double center;
    REQUIRE(intr.getParameter("principalPointX", center));
    REQUIRE(center == 2);

    // Mirroring reverses the output columns without changing the supplied frame.
    cv::Mat raw(4, 5, CV_8UC1, cv::Scalar(0));
    raw.at<unsigned char>(0, 0) = 42;
    REQUIRE(camera.setImage(raw).ok());
    REQUIRE(camera.setRgbMirroring(true).ok());
    REQUIRE(camera.getImage(image).ok());
    REQUIRE(image.at<unsigned char>(0, 4) == 42);

    // Extrinsics must be 4x4; reopening starts the sensor in standby.
    REQUIRE_FALSE(camera.setExtrinsicParam(cv::Mat::eye(3, 3, CV_64FC1)).ok());
    REQUIRE(camera.getSensorStatus() == dinrail::RGBDSensorStatus::OkStandby);
}

TEST_CASE("Camera diagnostic pattern is opt-in and refreshes its acquisition clock")
{
    // The default camera still returns simulator-driven black frames and zero stamps.
    dinrail::DrCameraFake camera;
    REQUIRE(camera.open({}));
    cv::Mat rgb;
    dinrail::Stamp stamp;
    REQUIRE(camera.getRgbImage(rgb, &stamp).ok());
    REQUIRE(cv::norm(rgb, cv::NORM_INF) == 0);
    REQUIRE(stamp.time == std::chrono::nanoseconds::zero());

    // The generated chart has RGB color bars and visible text in its clock panel.
    dinrail::Parameters config;
    config.put("test_pattern", true);
    REQUIRE(camera.open(config));
    REQUIRE(camera.getRgbImage(rgb, &stamp).ok());
    REQUIRE(stamp.time > std::chrono::nanoseconds::zero());
    REQUIRE(camera.getLastInputStamp().time == stamp.time);
    REQUIRE(rgb.at<cv::Vec3b>(200, 450) == cv::Vec3b(255, 0, 0));
    REQUIRE(cv::norm(rgb(cv::Rect(40, 415, 560, 50)), cv::NORM_INF) > 0);

    // Acquiring a pair refreshes RGB while leaving simulator depth untouched.
    cv::Mat depth;
    dinrail::Stamp nextStamp, depthStamp;
    REQUIRE(camera.getImages(rgb, depth, &nextStamp, &depthStamp).ok());
    REQUIRE(nextStamp.sequenceNumber == stamp.sequenceNumber + 1);
    REQUIRE(nextStamp.time != std::chrono::nanoseconds::zero());
    REQUIRE(cv::norm(depth, cv::NORM_INF) == 0);
    REQUIRE(depthStamp.time == std::chrono::nanoseconds::zero());

    // Raw acquisition provides a grayscale chart and updates precise timing too.
    cv::Mat mono;
    REQUIRE(camera.getImage(mono).ok());
    REQUIRE(mono.type() == CV_8UC1);
    REQUIRE(cv::norm(mono, cv::NORM_INF) > 0);
    REQUIRE(camera.getLastInputStamp().sequenceNumber == nextStamp.sequenceNumber + 1);

    // The same chart scales safely to small configured frame sizes.
    REQUIRE(camera.setRgbResolution(7, 3).ok());
    REQUIRE(camera.getRgbImage(rgb).ok());
    REQUIRE(rgb.cols == 7);
    REQUIRE(rgb.rows == 3);

    // Reopening without the option restores the default behavior.
    REQUIRE(camera.close());
    REQUIRE(camera.open({}));
    REQUIRE(camera.getRgbImage(rgb).ok());
    REQUIRE(cv::norm(rgb, cv::NORM_INF) == 0);
    config.put("test_pattern", "true");
    REQUIRE_FALSE(camera.open(config));
}
