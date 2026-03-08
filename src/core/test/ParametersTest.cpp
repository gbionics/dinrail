// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <chrono>
#include <vector>

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

    REQUIRE(params.find("device").as<std::string>() == "fakeMotionControl");
    REQUIRE(params.find("joints").as<int>() == 6);
    REQUIRE(params.find("period").as<double>() == 0.01);
    REQUIRE(params.find("enabled").as<bool>());

    REQUIRE(params.find("device").isString());
    REQUIRE(params.find("joints").isInt());
    REQUIRE(params.find("enabled").isBool());
    REQUIRE(params.find("period").isDouble());
    REQUIRE(params.find("missing").isNull());

    REQUIRE(params.find("enabled").asBool());
    REQUIRE(params.find("enabled").asInt() == static_cast<int>('1'));
    REQUIRE(params.find("enabled").asDouble() == static_cast<double>('1'));
    REQUIRE(params.find("device").asString() == "fakeMotionControl");

    REQUIRE(params.find("device").asBool() == false);
    REQUIRE(params.find("device").asInt() == 0);
    REQUIRE(params.find("device").asDouble() == 0.0);

    const dinrail::Value fallbackInt(99);
    REQUIRE(params.check("joints", fallbackInt).asInt() == 6);
    REQUIRE(params.check("missing", fallbackInt, "unused-comment").asInt() == 99);
}

TEST_CASE("Parameters supports nested groups", "[Parameters]")
{
    dinrail::Parameters params;

    dinrail::Parameters& general = params.addGroup("GENERAL");
    general.put("Joints", 4);

    dinrail::Parameters& network = params.addGroup("NETWORK");
    network.put("host", std::string("127.0.0.1"));
    network.put("port", 10000);

    const auto& generalRead = params.findGroup("GENERAL");
    REQUIRE_FALSE(generalRead.isNull());
    REQUIRE(generalRead.find("Joints").as<int>() == 4);

    const auto& networkRead = params.findGroup("NETWORK");
    REQUIRE_FALSE(networkRead.isNull());
    REQUIRE(networkRead.find("host").as<std::string>() == "127.0.0.1");
    REQUIRE(networkRead.find("port").as<int>() == 10000);
}

TEST_CASE("Parameters group returns existing group", "[Parameters]")
{
    dinrail::Parameters params;

    dinrail::Parameters& first = params.addGroup("GENERAL");
    first.put("Joints", 8);

    dinrail::Parameters& second = params.addGroup("GENERAL");
    REQUIRE(second.find("Joints").as<int>() == 8);

    second.put("Axes", 8);
    REQUIRE(first.find("Axes").as<int>() == 8);
}

TEST_CASE("Parameters copy is deep for groups", "[Parameters]")
{
    dinrail::Parameters original;
    original.put("device", std::string("test"));
    original.addGroup("GENERAL").put("Joints", 6);

    dinrail::Parameters copied = original;
    copied.addGroup("GENERAL").put("Joints", 12);

    const auto& originalGeneral = original.findGroup("GENERAL");
    REQUIRE_FALSE(originalGeneral.isNull());
    REQUIRE(originalGeneral.find("Joints").as<int>() == 6);

    const auto& copiedGeneral = copied.findGroup("GENERAL");
    REQUIRE_FALSE(copiedGeneral.isNull());
    REQUIRE(copiedGeneral.find("Joints").as<int>() == 12);
}

TEST_CASE("Parameters clear removes values and groups", "[Parameters]")
{
    dinrail::Parameters params;
    params.put("device", std::string("fakeMotionControl"));
    params.addGroup("GENERAL").put("Joints", 6);

    params.clear();

    REQUIRE_FALSE(params.check("device"));
    REQUIRE(params.findGroup("GENERAL").isNull());
    REQUIRE(params.getValueKeys().empty());
    REQUIRE(params.getGroupKeys().empty());
}

TEST_CASE("Parameters supports BLF-style scalar getParameter and setParameter", "[Parameters]")
{
    dinrail::Parameters params;

    params.setParameter("iterations", 12);
    params.setParameter("gain", 3.5);
    params.setParameter("name", std::string("controller"));
    params.setParameter("enabled", true);
    params.setParameter("period", std::chrono::nanoseconds(1000000));

    int iterations = 0;
    double gain = 0.0;
    std::string name;
    bool enabled = false;
    std::chrono::nanoseconds period(0);

    REQUIRE(params.getParameter("iterations", iterations));
    REQUIRE(params.getParameter("gain", gain));
    REQUIRE(params.getParameter("name", name));
    REQUIRE(params.getParameter("enabled", enabled));
    REQUIRE(params.getParameter("period", period));

    REQUIRE(iterations == 12);
    REQUIRE(gain == 3.5);
    REQUIRE(name == "controller");
    REQUIRE(enabled);
    REQUIRE(period == std::chrono::nanoseconds(1000000));
}

TEST_CASE("Parameters supports BLF-style generic vector set and get", "[Parameters]")
{
    dinrail::Parameters params;

    std::vector<int> sourceInt{1, 2, 3};
    std::vector<double> sourceDouble{1.2, 2.3, 3.4};
    std::vector<std::string> sourceString{"a", "b", "c"};
    std::vector<std::chrono::nanoseconds> sourceDuration{std::chrono::nanoseconds(1),
                                                         std::chrono::nanoseconds(2)};

    params.setParameter("ints", dinrail::GenericVector<const int>::Ref(sourceInt));
    params.setParameter("doubles", dinrail::GenericVector<const double>::Ref(sourceDouble));
    params.setParameter("strings", dinrail::GenericVector<const std::string>::Ref(sourceString));
    params.setParameter("durations",
                        dinrail::GenericVector<const std::chrono::nanoseconds>::Ref(
                            sourceDuration));

    std::vector<int> outInt;
    std::vector<double> outDouble;
    std::vector<std::string> outString;
    std::vector<std::chrono::nanoseconds> outDuration;

    REQUIRE(params.getParameter("ints", dinrail::GenericVector<int>::Ref(outInt)));
    REQUIRE(params.getParameter("doubles", dinrail::GenericVector<double>::Ref(outDouble)));
    REQUIRE(params.getParameter("strings", dinrail::GenericVector<std::string>::Ref(outString)));
    REQUIRE(
        params.getParameter("durations",
                            dinrail::GenericVector<std::chrono::nanoseconds>::Ref(outDuration)));

    REQUIRE(outInt == sourceInt);
    REQUIRE(outDouble == sourceDouble);
    REQUIRE(outString == sourceString);
    REQUIRE(outDuration == sourceDuration);

    std::array<int, 2> fixedInt{0, 0};
    REQUIRE_FALSE(params.getParameter("ints", dinrail::GenericVector<int>::Ref(fixedInt)));
}
