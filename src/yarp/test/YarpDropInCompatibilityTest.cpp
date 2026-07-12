// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/Parameters.h>
#include <dinrail/YarpNativeViewHelpers.h>

#include <yarp/dev/ControlBoardInterfaces.h>

#include <vector>

TEST_CASE("Open YARP fakeMotionControl via dinrail compatibility", "[yarp][compat]")
{
    // Open fakeMotionControl device installed by YARP via the dinrail
    // YARP drop-in compatibiltiy
    dinrail::Parameters opts;
    opts.put("device", "fakeMotionControl");
    opts.put("dinrail_device_type", "yarp");

    dinrail::Parameters& general = opts.addGroup("GENERAL");
    general.put("Joints", 3);

    dinrail::Device device;
    REQUIRE(device.open(opts));

    // Check that the dinrail IAxisInfo interface is available
    // and working, via the interface compatibility layer adapter
    dinrail::IAxisInfo* axisInfo = nullptr;
    REQUIRE(device.view(axisInfo));
    REQUIRE(axisInfo != nullptr);

    yarp::dev::IAxisInfo* nativeAxisInfo = nullptr;
    REQUIRE(device.view(nativeAxisInfo));
    REQUIRE(nativeAxisInfo != nullptr);

    int axes = 0;
    REQUIRE(axisInfo->getAxes(&axes));
    REQUIRE(axes == 3);

    int nativeAxes = 0;
    REQUIRE(nativeAxisInfo->getAxes(&nativeAxes));
    REQUIRE(nativeAxes == 3);

    dinrail::JointType axisType = dinrail::JointType::UNKNOWN;
    REQUIRE(axisInfo->getJointType(0, axisType));
    REQUIRE(axisType != dinrail::JointType::UNKNOWN);

    // Check that interfaces not currently exposed through dinrail compatibility
    // (like yarp::dev::IEncoders) are available
    yarp::dev::IEncoders* encoders = nullptr;
    REQUIRE(device.view(encoders));
    REQUIRE(encoders != nullptr);

    yarp::dev::DeviceDriver* nativeDriver = nullptr;
    REQUIRE(device.view(nativeDriver));
    REQUIRE(nativeDriver != nullptr);

    int encAxes = 0;
    REQUIRE(encoders->getAxes(&encAxes));
    REQUIRE(encAxes == 3);

    std::vector<double> initialEncoders(encAxes, 0.0);
    REQUIRE(encoders->getEncoders(initialEncoders.data()));

    for (int axis = 0; axis < encAxes; ++axis)
    {
        double encoder = 0.0;
        REQUIRE(encoders->getEncoder(axis, &encoder));
        REQUIRE(encoder == initialEncoders[static_cast<std::size_t>(axis)]);
    }

    yarp::dev::IEncoders* encodersViaHelper = nullptr;
    REQUIRE(dinrail::viewInterfaceInYARPDeviceOpenedViaDinrail(device, encodersViaHelper));
    REQUIRE(encodersViaHelper != nullptr);

    REQUIRE(device.close());
}