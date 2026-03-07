// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Parameters.h>

TEST_CASE("Parameters stores and retrieves scalar values", "[Parameters]")
{
    dinrail::Parameters params;

    params.put("device", std::string("fakeMotionControl"));
    params.put("joints", 6);
    params.put("period", 0.01);
    params.put("enabled", true);

    REQUIRE(params.check("device"));
    REQUIRE(params.check("joints"));
    REQUIRE(params.check("period"));
    REQUIRE(params.check("enabled"));

    REQUIRE(params.getString("device") == std::optional<std::string>{"fakeMotionControl"});
    REQUIRE(params.getInt64("joints") == std::optional<std::int64_t>{6});
    REQUIRE(params.getFloat64("period") == std::optional<double>{0.01});
    REQUIRE(params.getBool("enabled") == std::optional<bool>{true});

    REQUIRE_FALSE(params.getString("missing").has_value());
    REQUIRE_FALSE(params.getInt64("device").has_value());
}

TEST_CASE("Parameters supports nested groups", "[Parameters]")
{
    dinrail::Parameters params;

    dinrail::Parameters& general = params.addGroup("GENERAL");
    general.put("Joints", 4);

    dinrail::Parameters& network = params.addGroup("NETWORK");
    network.put("host", std::string("127.0.0.1"));
    network.put("port", 10000);

    const auto generalRead = params.findGroup("GENERAL");
    REQUIRE(generalRead.has_value());
    REQUIRE(generalRead->get().getInt64("Joints") == std::optional<std::int64_t>{4});

    const auto networkRead = params.findGroup("NETWORK");
    REQUIRE(networkRead.has_value());
    REQUIRE(networkRead->get().getString("host") == std::optional<std::string>{"127.0.0.1"});
    REQUIRE(networkRead->get().getInt64("port") == std::optional<std::int64_t>{10000});
}

TEST_CASE("Parameters group returns existing group", "[Parameters]")
{
    dinrail::Parameters params;

    dinrail::Parameters& first = params.addGroup("GENERAL");
    first.put("Joints", 8);

    dinrail::Parameters& second = params.addGroup("GENERAL");
    REQUIRE(second.getInt64("Joints") == std::optional<std::int64_t>{8});

    second.put("Axes", 8);
    REQUIRE(first.getInt64("Axes") == std::optional<std::int64_t>{8});
}

TEST_CASE("Parameters copy is deep for groups", "[Parameters]")
{
    dinrail::Parameters original;
    original.put("device", std::string("test"));
    original.addGroup("GENERAL").put("Joints", 6);

    dinrail::Parameters copied = original;
    copied.addGroup("GENERAL").put("Joints", 12);

    const auto originalGeneral = original.findGroup("GENERAL");
    REQUIRE(originalGeneral.has_value());
    REQUIRE(originalGeneral->get().getInt64("Joints") == std::optional<std::int64_t>{6});

    const auto copiedGeneral = copied.findGroup("GENERAL");
    REQUIRE(copiedGeneral.has_value());
    REQUIRE(copiedGeneral->get().getInt64("Joints") == std::optional<std::int64_t>{12});
}

TEST_CASE("Parameters clear removes values and groups", "[Parameters]")
{
    dinrail::Parameters params;
    params.put("device", std::string("fakeMotionControl"));
    params.addGroup("GENERAL").put("Joints", 6);

    params.clear();

    REQUIRE_FALSE(params.check("device"));
    REQUIRE_FALSE(params.findGroup("GENERAL").has_value());
    REQUIRE(params.getValueKeys().empty());
    REQUIRE(params.getGroupKeys().empty());
}
