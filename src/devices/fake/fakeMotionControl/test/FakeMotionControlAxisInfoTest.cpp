// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/Parameters.h>

TEST_CASE("FakeMotionControl exposes IAxisInfo", "[core][device][axisinfo]")
{
    dinrail::Parameters opts;
    opts.put("device", "fakeMotionControl");
    opts.put("number_of_joints", 3);
    opts.put("joint_names", std::vector<std::string>{"shoulder", "elbow", "wrist"});
    opts.put("joint_type", std::vector<std::string>{"revolute", "prismatic", "revolute"});

    dinrail::Device device;
    REQUIRE(device.open(opts));

    dinrail::IAxisInfo* axisInfo = nullptr;
    REQUIRE(device.view(axisInfo));
    REQUIRE(axisInfo != nullptr);

    int axes = 0;
    REQUIRE(axisInfo->getAxes(&axes));
    REQUIRE(axes == 3);

    std::string axisName;
    REQUIRE(axisInfo->getAxisName(0, axisName));
    REQUIRE(axisName == "shoulder");

    dinrail::JointType axisType = dinrail::JointType::UNKNOWN;
    REQUIRE(axisInfo->getJointType(0, axisType));
    REQUIRE(axisType == dinrail::JointType::REVOLUTE);
    REQUIRE(axisInfo->getJointType(1, axisType));
    REQUIRE(axisType == dinrail::JointType::PRISMATIC);

    REQUIRE(device.close());
}
