// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause
#include "../DrCameraFakeYarp.h"
#include <catch2/catch_test_macros.hpp>
#include <dinrail/Device.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Network.h>

TEST_CASE("Single YARP camera wrapper supports RGBD and framegrabber modes")
{
    // Configure one wrapper for both acquisition modes.
    yarp::os::Property config;
    config.put("device", "dr_camera_fake");
    config.put("width", 3);
    config.put("height", 2);
    config.put("depth_width", 3);
    config.put("depth_height", 2);

    // Exercise the same wrapper in RGBD and framegrabber modes.
    DrCameraFakeYarp wrapper;
    for (auto mode : {"rgbd", "framegrabber"})
    {
        config.put("mode", mode);
        REQUIRE(wrapper.open(config));

        // Supply RGB data through the wrapper's underlying native device.
        dinrail::IRGBDSensorSimulation* simulation = nullptr;
        REQUIRE(wrapper.device().view(simulation));
        cv::Mat rgb(2, 3, CV_8UC3, cv::Scalar(1, 2, 3));
        dinrail::Stamp stamp{std::chrono::nanoseconds(100000000), 6};
        REQUIRE(simulation->setRgbImage(rgb, stamp).ok());

        // View the YARP acquisition interfaces exposed by the wrapper.
        auto& color = static_cast<yarp::dev::IFrameGrabberImage&>(wrapper);
        auto& raw = static_cast<yarp::dev::IFrameGrabberImageRaw&>(wrapper);
        auto& sensor = static_cast<yarp::dev::IRGBDSensor&>(wrapper);

        // RGB channels retain their order, and raw acquisition is available in both modes.
        yarp::sig::ImageOf<yarp::sig::PixelRgb> colorImage;
        REQUIRE(color.getImage(colorImage));
        REQUIRE(colorImage.pixel(2, 1).r == 1);
        REQUIRE(colorImage.pixel(2, 1).b == 3);
        yarp::sig::ImageOf<yarp::sig::PixelMono> mono;
        REQUIRE(raw.getImage(mono));

        // Only RGBD mode permits depth acquisition.
        yarp::sig::ImageOf<yarp::sig::PixelFloat> depth;
        REQUIRE(sensor.getDepthImage(depth) == (std::string(mode) == "rgbd"));
        REQUIRE(wrapper.close());
    }

    // A failed open must release the underlying device.
    config.put("mode", "invalid");
    REQUIRE_FALSE(wrapper.open(config));
    REQUIRE_FALSE(wrapper.device().isValid());
    REQUIRE(wrapper.close());
}

TEST_CASE("YARP camera plugin can be opened through PolyDriver and dinrail interop")
{
    // Initialize YARP and load the installed wrapper through PolyDriver.
    yarp::os::Network network(yarp::os::YARP_CLOCK_SYSTEM);
    yarp::os::Property config;
    config.put("device", "dr_camera_fake");
    yarp::dev::PolyDriver driver;
    REQUIRE(driver.open(config));

    // The plugin exposes RGBD, visual parameters, controls and both framegrabbers.
    yarp::dev::IRGBDSensor* sensor = nullptr;
    REQUIRE(driver.view(sensor));
    yarp::dev::IRgbVisualParams* rgbParams = nullptr;
    REQUIRE(driver.view(rgbParams));
    yarp::dev::IFrameGrabberControls* controls = nullptr;
    REQUIRE(driver.view(controls));
    yarp::dev::IFrameGrabberImageRaw* raw = nullptr;
    REQUIRE(driver.view(raw));
    yarp::dev::IFrameGrabberImage* rgb = nullptr;
    REQUIRE(driver.view(rgb));
    REQUIRE(driver.close());

    // Open the YARP plugin through dinrail interop and acquire a native cv::Mat.
    dinrail::Parameters options;
    options.put("device", "dr_camera_fake");
    options.put("dinrail_device_type", "yarp");
    dinrail::Device device;
    REQUIRE(device.open(options));
    dinrail::IRGBDSensor* camera = nullptr;
    REQUIRE(device.view(camera));
    cv::Mat image;
    REQUIRE(camera->getRgbImage(image).ok());
    REQUIRE(image.type() == CV_8UC3);
    REQUIRE(device.close());
}
