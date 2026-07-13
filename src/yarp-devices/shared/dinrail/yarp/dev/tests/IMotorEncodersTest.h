/*
 * SPDX-FileCopyrightText: 2023-2023 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMOTORENCODERSTEST_H
#define IMOTORENCODERSTEST_H

#include <catch2/catch_test_macros.hpp>
#include <yarp/dev/IControlMode.h>
#include <yarp/dev/IMotorEncoders.h>

namespace yarp::dev::tests
{
inline void exec_iMotorEncoders_test_1(yarp::dev::IMotorEncoders* imotenc)
{
    REQUIRE(imotenc != nullptr);

    bool b = false;
    double value = 0;
    double time = 0;
    int axis = 0;
    b = imotenc->getNumberOfMotorEncoders(&axis);
    CHECK(b);
    CHECK(axis != 0);

    b = imotenc->getMotorEncoder(0, &value);
    CHECK(b);
    b = imotenc->getMotorEncoderAcceleration(0, &value);
    CHECK(b);
    b = imotenc->getMotorEncoderSpeed(0, &value);
    CHECK(b);
    b = imotenc->getMotorEncoderTimed(0, &value, &time);
    CHECK(b);
}
} // namespace yarp::dev::tests

#endif
