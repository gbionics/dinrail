// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Parameters.h>

#include <yarp/os/Value.h>

#include <array>
#include <cstdint>
#include <string>

namespace
{

yarp::os::Value makeYarpBool(const bool value)
{
    yarp::os::Value yarpValue;
    yarpValue.fromString(value ? "true" : "false");
    return yarpValue;
}

struct ValuePair
{
    yarp::os::Value yarp;
    dinrail::Value dinrail;
    std::string label;
};

} // namespace

TEST_CASE("yarp::os::Value and dinrail::Value scalar compatibility", "[ValueCompatibility]")
{
    const std::array<ValuePair, 4> values = {
        ValuePair{makeYarpBool(true), dinrail::Value(true), "bool"},
        ValuePair{yarp::os::Value(static_cast<std::int32_t>(42)), dinrail::Value(42), "int"},
        ValuePair{yarp::os::Value(3.5), dinrail::Value(3.5), "double"},
        ValuePair{yarp::os::Value(std::string("hello")), dinrail::Value(std::string("hello")), "string"},
    };

    for (const auto& valuePair : values)
    {
        INFO("Checking source type: " << valuePair.label);

        REQUIRE(valuePair.yarp.isBool() == valuePair.dinrail.isBool());
        REQUIRE(valuePair.yarp.isInt32() == valuePair.dinrail.isInt());
        REQUIRE(valuePair.yarp.isFloat64() == valuePair.dinrail.isDouble());
        REQUIRE(valuePair.yarp.isString() == valuePair.dinrail.isString());

        // Compare all scalar conversion pairs (bool/int/double/string).
        REQUIRE(valuePair.yarp.asBool() == valuePair.dinrail.asBool());
        REQUIRE(valuePair.yarp.asInt32() == valuePair.dinrail.asInt());
        REQUIRE(valuePair.yarp.asFloat64() == valuePair.dinrail.asDouble());
        REQUIRE(valuePair.yarp.asString() == valuePair.dinrail.asString());
    }
}

TEST_CASE("yarp::os::Value and dinrail::Value null compatibility", "[ValueCompatibility]")
{
    const yarp::os::Value& yarpNull = yarp::os::Value::getNullValue();
    const dinrail::Value dinrailNull;

    REQUIRE(yarpNull.isNull() == dinrailNull.isNull());
    REQUIRE(yarpNull.asBool() == dinrailNull.asBool());
    REQUIRE(yarpNull.asInt32() == dinrailNull.asInt());
    REQUIRE(yarpNull.asFloat64() == dinrailNull.asDouble());
    REQUIRE(yarpNull.asString() == dinrailNull.asString());
}
