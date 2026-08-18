// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/YarpStampConverter.h>

#include <yarp/os/Stamp.h>

#include <chrono>
#include <cstdint>

TEST_CASE("YARP stamp conversion preserves sequence number", "[YarpStampConverter]")
{
    const dinrail::Stamp dinrailStamp{std::chrono::nanoseconds{123456789}, 17};

    const yarp::os::Stamp yarpStamp = dinrail::toYarpStamp(dinrailStamp);
    REQUIRE(yarpStamp.getCount() == 17);

    const dinrail::Stamp convertedStamp = dinrail::toDinrailStamp(yarpStamp);
    REQUIRE(convertedStamp.sequenceNumber == static_cast<std::int32_t>(17));
    REQUIRE(convertedStamp.time == dinrailStamp.time);
}

TEST_CASE("YARP stamp conversion handles fractional seconds", "[YarpStampConverter]")
{
    const yarp::os::Stamp yarpStamp{23, 12.345678};

    const dinrail::Stamp dinrailStamp = dinrail::toDinrailStamp(yarpStamp);

    REQUIRE(dinrailStamp.sequenceNumber == static_cast<std::int32_t>(23));
    REQUIRE(dinrailStamp.time == std::chrono::nanoseconds{12345678000});
}
