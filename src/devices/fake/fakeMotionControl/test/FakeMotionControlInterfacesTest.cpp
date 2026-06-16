// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/Parameters.h>

#include <vector>

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

TEST_CASE("FakeMotionControl exposes IImpedanceAllSetPointsControl",
          "[core][device][impedance_all_setpoints]")
{
    dinrail::Parameters opts;
    opts.put("device", "fakeMotionControl");
    opts.put("number_of_joints", 3);

    dinrail::Device device;
    REQUIRE(device.open(opts));

    dinrail::IImpedanceAllSetPointsControl* impedanceAllSetPoints = nullptr;
    REQUIRE(device.view(impedanceAllSetPoints));
    REQUIRE(impedanceAllSetPoints != nullptr);

    SECTION("single joint setpoint")
    {
        REQUIRE(impedanceAllSetPoints->setSetPoint(0, 1.0, 2.0, 3.0, 4.0, 5.0));

        double pos = 0.0;
        double vel = 0.0;
        double torque = 0.0;
        double stiffness = 0.0;
        double damping = 0.0;
        REQUIRE(impedanceAllSetPoints->getSetPoint(0, pos, vel, torque, stiffness, damping));
        REQUIRE(pos == 1.0);
        REQUIRE(vel == 2.0);
        REQUIRE(torque == 3.0);
        REQUIRE(stiffness == 4.0);
        REQUIRE(damping == 5.0);

        REQUIRE_FALSE(impedanceAllSetPoints->setSetPoint(3, 1.0, 2.0, 3.0, 4.0, 5.0));
        REQUIRE_FALSE(impedanceAllSetPoints->setSetPoint(-1, 1.0, 2.0, 3.0, 4.0, 5.0));
        REQUIRE_FALSE(impedanceAllSetPoints->getSetPoint(3, pos, vel, torque, stiffness, damping));
        REQUIRE_FALSE(impedanceAllSetPoints->getSetPoint(-1, pos, vel, torque, stiffness, damping));
    }

    SECTION("subset setpoints")
    {
        const std::vector<int> joints{0, 2};
        const std::vector<double> pos{1.0, 2.0};
        const std::vector<double> vel{3.0, 4.0};
        const std::vector<double> torque{5.0, 6.0};
        const std::vector<double> stiffness{7.0, 8.0};
        const std::vector<double> damping{9.0, 10.0};

        REQUIRE(impedanceAllSetPoints->setSetPoints(joints, pos, vel, torque, stiffness, damping));

        std::vector<double> posOut{0.0, 0.0};
        std::vector<double> velOut{0.0, 0.0};
        std::vector<double> torqueOut{0.0, 0.0};
        std::vector<double> stiffnessOut{0.0, 0.0};
        std::vector<double> dampingOut{0.0, 0.0};
        REQUIRE(impedanceAllSetPoints->getSetPoints(
            joints, posOut, velOut, torqueOut, stiffnessOut, dampingOut));
        REQUIRE(posOut == pos);
        REQUIRE(velOut == vel);
        REQUIRE(torqueOut == torque);
        REQUIRE(stiffnessOut == stiffness);
        REQUIRE(dampingOut == damping);

        const std::vector<double> shortVec{1.0};
        REQUIRE_FALSE(
            impedanceAllSetPoints->setSetPoints(joints, shortVec, vel, torque, stiffness, damping));
        std::vector<double> shortOut{0.0};
        REQUIRE_FALSE(impedanceAllSetPoints->getSetPoints(
            joints, posOut, shortOut, torqueOut, stiffnessOut, dampingOut));

        const std::vector<int> invalidJoints{0, 3};
        REQUIRE_FALSE(
            impedanceAllSetPoints->setSetPoints(invalidJoints, pos, vel, torque, stiffness, damping));
        REQUIRE_FALSE(impedanceAllSetPoints->getSetPoints(
            invalidJoints, posOut, velOut, torqueOut, stiffnessOut, dampingOut));
    }

    SECTION("all joints setpoints")
    {
        const std::vector<double> pos{1.0, 2.0, 3.0};
        const std::vector<double> vel{4.0, 5.0, 6.0};
        const std::vector<double> torque{7.0, 8.0, 9.0};
        const std::vector<double> stiffness{10.0, 11.0, 12.0};
        const std::vector<double> damping{13.0, 14.0, 15.0};

        REQUIRE(impedanceAllSetPoints->setSetPoints(pos, vel, torque, stiffness, damping));

        std::vector<double> posOut{0.0, 0.0, 0.0};
        std::vector<double> velOut{0.0, 0.0, 0.0};
        std::vector<double> torqueOut{0.0, 0.0, 0.0};
        std::vector<double> stiffnessOut{0.0, 0.0, 0.0};
        std::vector<double> dampingOut{0.0, 0.0, 0.0};
        REQUIRE(impedanceAllSetPoints->getSetPoints(posOut, velOut, torqueOut, stiffnessOut, dampingOut));
        REQUIRE(posOut == pos);
        REQUIRE(velOut == vel);
        REQUIRE(torqueOut == torque);
        REQUIRE(stiffnessOut == stiffness);
        REQUIRE(dampingOut == damping);

        const std::vector<double> wrongSize{1.0, 2.0};
        REQUIRE_FALSE(impedanceAllSetPoints->setSetPoints(
            wrongSize, vel, torque, stiffness, damping));
        std::vector<double> wrongSizeOut{0.0, 0.0};
        REQUIRE_FALSE(impedanceAllSetPoints->getSetPoints(
            wrongSizeOut, velOut, torqueOut, stiffnessOut, dampingOut));
    }

    REQUIRE(device.close());
}
