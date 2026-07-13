/*
 * SPDX-FileCopyrightText: 2026-2026 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IIMPEDANCEALLSETPOINTSCONTROLTEST_H
#define IIMPEDANCEALLSETPOINTSCONTROLTEST_H

#include <dinrail/IImpedanceAllSetPointsControl.h>

#include <yarp/dev/IAxisInfo.h>
#include <yarp/os/Time.h>

#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

namespace yarp::dev::tests
{
inline void
exec_iImpedanceAllSetPointsControl_test_1(dinrail::IImpedanceAllSetPointsControl* iimpAll,
                                          yarp::dev::IAxisInfo* iinfo)
{
    const auto closeEnough = [](double a, double b) { return std::fabs(a - b) < 1e-12; };

    const auto waitUntil = [](auto&& predicate) {
        constexpr double timeout = 1.0;
        constexpr double sleepStep = 0.001;
        const double deadline = yarp::os::Time::now() + timeout;
        while (yarp::os::Time::now() < deadline)
        {
            if (predicate())
            {
                return true;
            }
            yarp::os::Time::delay(sleepStep);
        }
        return predicate();
    };

    REQUIRE(iimpAll != nullptr);
    REQUIRE(iinfo != nullptr);

    int axes = 0;
    REQUIRE(iinfo->getAxes(&axes));
    REQUIRE(axes > 0);

    {
        const bool ok = iimpAll->setSetPoint(0, 1.0, 2.0, 3.0, 4.0, 5.0);
        CHECK(ok);

        double pos = 0.0;
        double vel = 0.0;
        double torque = 0.0;
        double stiffness = 0.0;
        double damping = 0.0;
        REQUIRE(waitUntil([&]() {
            return iimpAll->getSetPoint(0, pos, vel, torque, stiffness, damping)
                   && closeEnough(pos, 1.0) && closeEnough(vel, 2.0) && closeEnough(torque, 3.0)
                   && closeEnough(stiffness, 4.0) && closeEnough(damping, 5.0);
        }));
        CHECK(pos == 1.0);
        CHECK(vel == 2.0);
        CHECK(torque == 3.0);
        CHECK(stiffness == 4.0);
        CHECK(damping == 5.0);
    }

    if (axes > 1)
    {
        std::vector<int> joints{0, 1};
        std::vector<double> pos{10.0, 11.0};
        std::vector<double> vel{12.0, 13.0};
        std::vector<double> torque{14.0, 15.0};
        std::vector<double> stiffness{16.0, 17.0};
        std::vector<double> damping{18.0, 19.0};

        REQUIRE(iimpAll->setSetPoints(joints, pos, vel, torque, stiffness, damping));

        std::vector<double> gotPos(joints.size(), 0.0);
        std::vector<double> gotVel(joints.size(), 0.0);
        std::vector<double> gotTorque(joints.size(), 0.0);
        std::vector<double> gotStiffness(joints.size(), 0.0);
        std::vector<double> gotDamping(joints.size(), 0.0);

        REQUIRE(waitUntil([&]() {
            return iimpAll->getSetPoints(joints, gotPos, gotVel, gotTorque, gotStiffness, gotDamping)
                   && closeEnough(gotPos[0], 10.0) && closeEnough(gotPos[1], 11.0)
                   && closeEnough(gotVel[0], 12.0) && closeEnough(gotVel[1], 13.0)
                   && closeEnough(gotTorque[0], 14.0) && closeEnough(gotTorque[1], 15.0)
                   && closeEnough(gotStiffness[0], 16.0) && closeEnough(gotStiffness[1], 17.0)
                   && closeEnough(gotDamping[0], 18.0) && closeEnough(gotDamping[1], 19.0);
        }));
        CHECK(gotPos[0] == 10.0);
        CHECK(gotPos[1] == 11.0);
        CHECK(gotVel[0] == 12.0);
        CHECK(gotVel[1] == 13.0);
        CHECK(gotTorque[0] == 14.0);
        CHECK(gotTorque[1] == 15.0);
        CHECK(gotStiffness[0] == 16.0);
        CHECK(gotStiffness[1] == 17.0);
        CHECK(gotDamping[0] == 18.0);
        CHECK(gotDamping[1] == 19.0);
    }

    {
        std::vector<double> pos(static_cast<std::size_t>(axes), 20.0);
        std::vector<double> vel(static_cast<std::size_t>(axes), 21.0);
        std::vector<double> torque(static_cast<std::size_t>(axes), 22.0);
        std::vector<double> stiffness(static_cast<std::size_t>(axes), 23.0);
        std::vector<double> damping(static_cast<std::size_t>(axes), 24.0);

        REQUIRE(iimpAll->setSetPoints(pos, vel, torque, stiffness, damping));

        std::vector<double> gotPos(static_cast<std::size_t>(axes), 0.0);
        std::vector<double> gotVel(static_cast<std::size_t>(axes), 0.0);
        std::vector<double> gotTorque(static_cast<std::size_t>(axes), 0.0);
        std::vector<double> gotStiffness(static_cast<std::size_t>(axes), 0.0);
        std::vector<double> gotDamping(static_cast<std::size_t>(axes), 0.0);

        REQUIRE(waitUntil([&]() {
            if (!iimpAll->getSetPoints(gotPos, gotVel, gotTorque, gotStiffness, gotDamping))
            {
                return false;
            }

            for (int j = 0; j < axes; ++j)
            {
                const std::size_t idx = static_cast<std::size_t>(j);
                if (!closeEnough(gotPos[idx], 20.0) || !closeEnough(gotVel[idx], 21.0)
                    || !closeEnough(gotTorque[idx], 22.0) || !closeEnough(gotStiffness[idx], 23.0)
                    || !closeEnough(gotDamping[idx], 24.0))
                {
                    return false;
                }
            }

            return true;
        }));

        for (int j = 0; j < axes; ++j)
        {
            CHECK(gotPos[static_cast<std::size_t>(j)] == 20.0);
            CHECK(gotVel[static_cast<std::size_t>(j)] == 21.0);
            CHECK(gotTorque[static_cast<std::size_t>(j)] == 22.0);
            CHECK(gotStiffness[static_cast<std::size_t>(j)] == 23.0);
            CHECK(gotDamping[static_cast<std::size_t>(j)] == 24.0);
        }
    }
}
} // namespace yarp::dev::tests

#endif
