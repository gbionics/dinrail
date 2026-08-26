// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/IPreciselyTimed.h>
#include <dinrail/Parameters.h>

#include <array>
#include <chrono>
#include <vector>

TEST_CASE("FakeMotionControl exposes encoder interfaces", "[core][device][encoders]")
{
    dinrail::Parameters opts;
    opts.put("device", "dr_controlboard_fake");
    opts.put("number_of_joints", 3);

    dinrail::Device device;
    REQUIRE(device.open(opts));

    dinrail::IEncoders* encoders = nullptr;
    REQUIRE(device.view(encoders));
    REQUIRE(encoders != nullptr);

    dinrail::IEncodersSimulation* encoderSimulation = nullptr;
    REQUIRE(device.view(encoderSimulation));
    REQUIRE(encoderSimulation != nullptr);

    int axes = 0;
    REQUIRE(encoders->getAxes(&axes));
    REQUIRE(axes == 3);

    SECTION("single encoder measurements")
    {
        REQUIRE(encoderSimulation->setEncoder(1, 11.5));
        double value = 0.0;
        REQUIRE(encoders->getEncoder(1, &value));
        REQUIRE(value == 11.5);

        REQUIRE(encoderSimulation->setEncoderTimed(1, 12.5, 42.25));
        REQUIRE(encoderSimulation->setEncoderSpeed(1, 3.5));
        REQUIRE(encoderSimulation->setEncoderAcceleration(1, -1.25));

        double timestamp = 0.0;
        double speed = 0.0;
        double acceleration = 0.0;
        REQUIRE(encoders->getEncoder(1, &value));
        REQUIRE(value == 12.5);
        REQUIRE(encoders->getEncoderTimed(1, &value, &timestamp));
        REQUIRE(value == 12.5);
        REQUIRE(timestamp == 42.25);
        REQUIRE(encoders->getEncoderSpeed(1, &speed));
        REQUIRE(speed == 3.5);
        REQUIRE(encoders->getEncoderAcceleration(1, &acceleration));
        REQUIRE(acceleration == -1.25);

        REQUIRE_FALSE(encoderSimulation->setEncoder(-1, 0.0));
        REQUIRE_FALSE(encoderSimulation->setEncoder(3, 0.0));
        REQUIRE_FALSE(encoders->getEncoder(-1, &value));
        REQUIRE_FALSE(encoders->getEncoder(3, &value));
        REQUIRE_FALSE(encoders->getEncoder(0, nullptr));
        REQUIRE_FALSE(encoders->getEncoderTimed(0, nullptr, &timestamp));
        REQUIRE_FALSE(encoders->getEncoderTimed(0, &value, nullptr));
        REQUIRE_FALSE(encoders->getEncoderSpeed(0, nullptr));
        REQUIRE_FALSE(encoders->getEncoderAcceleration(0, nullptr));
    }

    SECTION("all encoder measurements")
    {
        const std::vector<double> values{1.0, 2.0, 3.0};
        const std::vector<double> timestamps{10.0, 20.0, 30.0};
        const std::vector<double> speeds{4.0, 5.0, 6.0};
        const std::vector<double> accelerations{7.0, 8.0, 9.0};
        REQUIRE(encoderSimulation->setEncoders(values));
        std::vector<double> valuesOut;
        REQUIRE(encoders->getEncoders(valuesOut));
        REQUIRE(valuesOut == values);

        REQUIRE(encoderSimulation->setEncodersTimed(values, timestamps));
        REQUIRE(encoderSimulation->setEncoderSpeeds(speeds));
        REQUIRE(encoderSimulation->setEncoderAccelerations(accelerations));

        // Resizable outputs are resized to the number of axes.
        std::vector<double> timestampsOut;
        std::vector<double> speedsOut;
        std::vector<double> accelerationsOut;
        REQUIRE(encoders->getEncoders(valuesOut));
        REQUIRE(valuesOut == values);
        REQUIRE(encoders->getEncodersTimed(valuesOut, timestampsOut));
        REQUIRE(valuesOut == values);
        REQUIRE(timestampsOut == timestamps);
        REQUIRE(encoders->getEncoderSpeeds(speedsOut));
        REQUIRE(speedsOut == speeds);
        REQUIRE(encoders->getEncoderAccelerations(accelerationsOut));
        REQUIRE(accelerationsOut == accelerations);

        // Fixed-size outputs must already have the number of controlled axes.
        std::array<double, 3> fixedOutput{};
        REQUIRE(encoders->getEncoders(fixedOutput));
        REQUIRE(std::vector<double>(fixedOutput.begin(), fixedOutput.end()) == values);
        std::array<double, 2> shortOutput{};
        REQUIRE_FALSE(encoders->getEncoders(shortOutput));

        const std::vector<double> wrongSize{1.0, 2.0};
        REQUIRE_FALSE(encoderSimulation->setEncoders(wrongSize));
        REQUIRE_FALSE(encoderSimulation->setEncodersTimed(values, wrongSize));
        REQUIRE_FALSE(encoderSimulation->setEncoderSpeeds(wrongSize));
        REQUIRE_FALSE(encoderSimulation->setEncoderAccelerations(wrongSize));
    }

    REQUIRE(device.close());
}

TEST_CASE("FakeMotionControl exposes precisely timed interfaces", "[core][device][precisely_timed]")
{
    dinrail::Parameters opts;
    opts.put("device", "dr_controlboard_fake");

    dinrail::Device device;
    REQUIRE(device.open(opts));

    dinrail::IPreciselyTimed* preciselyTimed = nullptr;
    REQUIRE(device.view(preciselyTimed));
    REQUIRE(preciselyTimed != nullptr);

    dinrail::IPreciselyTimedSimulation* preciselyTimedSimulation = nullptr;
    REQUIRE(device.view(preciselyTimedSimulation));
    REQUIRE(preciselyTimedSimulation != nullptr);

    const auto initialStamp = preciselyTimed->getLastInputStamp();
    REQUIRE(initialStamp.time == std::chrono::nanoseconds::zero());
    REQUIRE(initialStamp.sequenceNumber == 0);

    const dinrail::Stamp expectedStamp{std::chrono::nanoseconds{123456789}, 42};
    preciselyTimedSimulation->setLastInputStamp(expectedStamp);

    const auto actualStamp = preciselyTimed->getLastInputStamp();
    REQUIRE(actualStamp.time == expectedStamp.time);
    REQUIRE(actualStamp.sequenceNumber == expectedStamp.sequenceNumber);

    REQUIRE(device.close());
}

TEST_CASE("FakeMotionControl exposes IAxisInfo", "[core][device][axisinfo]")
{
    dinrail::Parameters opts;
    opts.put("device", "dr_controlboard_fake");
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
    opts.put("device", "dr_controlboard_fake");
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
        REQUIRE(impedanceAllSetPoints
                    ->getSetPoints(joints, posOut, velOut, torqueOut, stiffnessOut, dampingOut));
        REQUIRE(posOut == pos);
        REQUIRE(velOut == vel);
        REQUIRE(torqueOut == torque);
        REQUIRE(stiffnessOut == stiffness);
        REQUIRE(dampingOut == damping);

        const std::vector<double> shortVec{1.0};
        REQUIRE_FALSE(
            impedanceAllSetPoints->setSetPoints(joints, shortVec, vel, torque, stiffness, damping));
        std::vector<double> shortOut{0.0};
        REQUIRE_FALSE(
            impedanceAllSetPoints
                ->getSetPoints(joints, posOut, shortOut, torqueOut, stiffnessOut, dampingOut));

        const std::vector<int> invalidJoints{0, 3};
        REQUIRE_FALSE(impedanceAllSetPoints
                          ->setSetPoints(invalidJoints, pos, vel, torque, stiffness, damping));
        REQUIRE_FALSE(
            impedanceAllSetPoints
                ->getSetPoints(invalidJoints, posOut, velOut, torqueOut, stiffnessOut, dampingOut));
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
        REQUIRE(impedanceAllSetPoints
                    ->getSetPoints(posOut, velOut, torqueOut, stiffnessOut, dampingOut));
        REQUIRE(posOut == pos);
        REQUIRE(velOut == vel);
        REQUIRE(torqueOut == torque);
        REQUIRE(stiffnessOut == stiffness);
        REQUIRE(dampingOut == damping);

        const std::vector<double> wrongSize{1.0, 2.0};
        REQUIRE_FALSE(
            impedanceAllSetPoints->setSetPoints(wrongSize, vel, torque, stiffness, damping));
        std::vector<double> wrongSizeOut{0.0, 0.0};
        REQUIRE_FALSE(impedanceAllSetPoints->getSetPoints(wrongSizeOut,
                                                          velOut,
                                                          torqueOut,
                                                          stiffnessOut,
                                                          dampingOut));
    }

    REQUIRE(device.close());
}
