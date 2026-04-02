// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/YarpPropertyConverter.h>
#include <dinrail/Parameters.h>

#include <yarp/os/Property.h>

#include <chrono>

TEST_CASE("YarpPropertyConverter converts device name", "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    dinrailProp.put("device", "fakeMotionControl");

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE(yarpProp.check("device"));
    REQUIRE(yarpProp.find("device").asString() == "fakeMotionControl");
}

TEST_CASE("YarpPropertyConverter handles missing device name", "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    // No device property set

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE_FALSE(yarpProp.check("device"));
}

TEST_CASE("YarpPropertyConverter converts GENERAL group with Joints parameter",
          "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    dinrailProp.put("device", "fakeMotionControl");

    dinrail::Parameters& general = dinrailProp.addGroup("GENERAL");
    general.put("Joints", 6);

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE(yarpProp.check("device"));
    REQUIRE(yarpProp.find("device").asString() == "fakeMotionControl");

    REQUIRE(yarpProp.check("GENERAL"));
    yarp::os::Bottle* generalGroup = yarpProp.find("GENERAL").asList();
    REQUIRE(generalGroup != nullptr);

    // YARP's Property groups are represented as Bottles
    // The group should contain "Joints" key and its value
    REQUIRE(generalGroup->size() >= 2);
    REQUIRE(generalGroup->get(0).asString() == "Joints");
    REQUIRE(generalGroup->get(1).asInt32() == 6);
}

TEST_CASE("YarpPropertyConverter handles missing GENERAL group", "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    dinrailProp.put("device", "testDevice");
    // No GENERAL group

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE(yarpProp.check("device"));
    REQUIRE_FALSE(yarpProp.check("GENERAL"));
}

TEST_CASE("YarpPropertyConverter handles GENERAL group without Joints", "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    dinrailProp.put("device", "testDevice");

    dinrail::Parameters& general = dinrailProp.addGroup("GENERAL");
    // GENERAL group exists but no Joints parameter

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE(yarpProp.check("device"));
    REQUIRE(yarpProp.check("GENERAL"));

    // When GENERAL group is empty (no properties set), YARP may return it differently
    // The important thing is that the group exists but doesn't contain Joints
    yarp::os::Bottle* generalGroup = yarpProp.find("GENERAL").asList();

    if (generalGroup != nullptr)
    {
        // Group should not contain Joints
        bool hasJoints = false;
        for (size_t i = 0; i < generalGroup->size(); ++i)
        {
            if (generalGroup->get(i).asString() == "Joints")
            {
                hasJoints = true;
                break;
            }
        }
        REQUIRE_FALSE(hasJoints);
    }
    // If generalGroup is nullptr, that's also acceptable - it means empty group
}

TEST_CASE("YarpPropertyConverter full configuration example", "[YarpPropertyConverter]")
{
    // Create a typical device configuration
    dinrail::Parameters dinrailProp;
    dinrailProp.put("device", "fakeMotionControl");

    dinrail::Parameters& general = dinrailProp.addGroup("GENERAL");
    general.put("Joints", 4);

    // Convert to YARP property
    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    // Verify device name
    REQUIRE(yarpProp.check("device"));
    REQUIRE(yarpProp.find("device").asString() == "fakeMotionControl");

    // Verify GENERAL group
    REQUIRE(yarpProp.check("GENERAL"));
    yarp::os::Bottle* generalGroup = yarpProp.find("GENERAL").asList();
    REQUIRE(generalGroup != nullptr);
    REQUIRE(generalGroup->size() >= 2);

    // Find Joints in the group
    int joints = -1;
    for (size_t i = 0; i < generalGroup->size() - 1; ++i)
    {
        if (generalGroup->get(i).asString() == "Joints")
        {
            joints = generalGroup->get(i + 1).asInt32();
            break;
        }
    }
    REQUIRE(joints == 4);
}

TEST_CASE("YarpPropertyConverter converts all data types", "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    dinrailProp.put("stringParam", "testString");
    dinrailProp.put("intParam", 42);
    dinrailProp.put("doubleParam", 3.14159);

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE(yarpProp.check("stringParam"));
    REQUIRE(yarpProp.find("stringParam").asString() == "testString");

    REQUIRE(yarpProp.check("intParam"));
    REQUIRE(yarpProp.find("intParam").asInt32() == 42);

    REQUIRE(yarpProp.check("doubleParam"));
    REQUIRE(yarpProp.find("doubleParam").asFloat64() == 3.14159);
}

TEST_CASE("YarpPropertyConverter converts vector of doubles", "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    const std::vector<double> expectedValues{0.1, -2.5, 3.75};
    dinrailProp.put("doubleVector", expectedValues);

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE(yarpProp.check("doubleVector"));

    const yarp::os::Value& convertedValue = yarpProp.find("doubleVector");
    REQUIRE(convertedValue.isList());

    yarp::os::Bottle* convertedList = convertedValue.asList();
    REQUIRE(convertedList != nullptr);
    REQUIRE(convertedList->size() == static_cast<int>(expectedValues.size()));

    for (int i = 0; i < convertedList->size(); ++i)
    {
        REQUIRE(convertedList->get(i).isFloat64());
        REQUIRE(convertedList->get(i).asFloat64() == expectedValues[static_cast<size_t>(i)]);
    }
}

TEST_CASE("YarpPropertyConverter converts all supported Parameters types",
          "[YarpPropertyConverter]")
{
    using namespace std::chrono_literals;

    dinrail::Parameters dinrailProp;
    dinrailProp.put("boolParam", true);
    dinrailProp.put("intParam", 10);
    dinrailProp.put("doubleParam", 2.5);
    dinrailProp.put("stringParam", "value");
    dinrailProp.put("durationParam", 12s + 345ms);

    dinrailProp.put("boolVector", std::vector<bool>{true, false, true});
    dinrailProp.put("intVector", std::vector<int>{1, 2, 3});
    dinrailProp.put("doubleVector", std::vector<double>{1.0, 2.0, 3.0});
    dinrailProp.put("stringVector", std::vector<std::string>{"a", "b"});
    dinrailProp.put("durationVector",
                    std::vector<std::chrono::nanoseconds>{1s + 1ms, 2s + 2ms, 3s + 3ms});

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE(yarpProp.find("boolParam").asInt32() == 1);
    REQUIRE(yarpProp.find("intParam").asInt32() == 10);
    REQUIRE(yarpProp.find("doubleParam").asFloat64() == 2.5);
    REQUIRE(yarpProp.find("stringParam").asString() == "value");
    REQUIRE(yarpProp.find("durationParam").asString() == "00:00:12:345000");

    yarp::os::Bottle* boolVector = yarpProp.find("boolVector").asList();
    REQUIRE(boolVector != nullptr);
    REQUIRE(boolVector->size() == 3);
    REQUIRE(boolVector->get(0).isBool());
    REQUIRE(boolVector->get(0).asBool() == true);
    REQUIRE(boolVector->get(1).asBool() == false);
    REQUIRE(boolVector->get(2).asBool() == true);

    yarp::os::Bottle* intVector = yarpProp.find("intVector").asList();
    REQUIRE(intVector != nullptr);
    REQUIRE(intVector->size() == 3);
    REQUIRE(intVector->get(0).asInt32() == 1);
    REQUIRE(intVector->get(1).asInt32() == 2);
    REQUIRE(intVector->get(2).asInt32() == 3);

    yarp::os::Bottle* doubleVector = yarpProp.find("doubleVector").asList();
    REQUIRE(doubleVector != nullptr);
    REQUIRE(doubleVector->size() == 3);
    REQUIRE(doubleVector->get(0).asFloat64() == 1.0);
    REQUIRE(doubleVector->get(1).asFloat64() == 2.0);
    REQUIRE(doubleVector->get(2).asFloat64() == 3.0);

    yarp::os::Bottle* stringVector = yarpProp.find("stringVector").asList();
    REQUIRE(stringVector != nullptr);
    REQUIRE(stringVector->size() == 2);
    REQUIRE(stringVector->get(0).asString() == "a");
    REQUIRE(stringVector->get(1).asString() == "b");

    yarp::os::Bottle* durationVector = yarpProp.find("durationVector").asList();
    REQUIRE(durationVector != nullptr);
    REQUIRE(durationVector->size() == 3);
    REQUIRE(durationVector->get(0).asString() == "00:00:01:001000");
    REQUIRE(durationVector->get(1).asString() == "00:00:02:002000");
    REQUIRE(durationVector->get(2).asString() == "00:00:03:003000");
}

TEST_CASE("YarpPropertyConverter converts multiple groups", "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    dinrailProp.put("device", "testDevice");

    dinrail::Parameters& group1 = dinrailProp.addGroup("GROUP1");
    group1.put("param1", 10);
    group1.put("param2", "value2");

    dinrail::Parameters& group2 = dinrailProp.addGroup("GROUP2");
    group2.put("param3", 20.5);
    group2.put("param4", "value4");

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    REQUIRE(yarpProp.check("device"));
    REQUIRE(yarpProp.check("GROUP1"));
    REQUIRE(yarpProp.check("GROUP2"));

    // Verify GROUP1
    yarp::os::Bottle group1Bottle = yarpProp.findGroup("GROUP1");
    REQUIRE(!group1Bottle.isNull());
    REQUIRE(group1Bottle.size() > 0);
    REQUIRE(group1Bottle.get(0).asString() == "GROUP1");

    bool foundParam1 = false;
    bool foundParam2 = false;
    // Start from index 1 since index 0 is the group name
    for (size_t i = 1; i < group1Bottle.size(); ++i)
    {
        yarp::os::Value& v = group1Bottle.get(i);
        if (v.isList())
        {
            yarp::os::Bottle* b = v.asList();
            if (b && b->size() >= 2)
            {
                std::string key = b->get(0).asString();
                if (key == "param1")
                {
                    REQUIRE(b->get(1).asInt32() == 10);
                    foundParam1 = true;
                } else if (key == "param2")
                {
                    REQUIRE(b->get(1).asString() == "value2");
                    foundParam2 = true;
                }
            }
        }
    }
    REQUIRE(foundParam1);
    REQUIRE(foundParam2);

    // Verify GROUP2
    yarp::os::Bottle group2Bottle = yarpProp.findGroup("GROUP2");
    REQUIRE(!group2Bottle.isNull());
    REQUIRE(group2Bottle.size() > 0);
    REQUIRE(group2Bottle.get(0).asString() == "GROUP2");

    bool foundParam3 = false;
    bool foundParam4 = false;
    for (size_t i = 1; i < group2Bottle.size(); ++i)
    {
        yarp::os::Value& v = group2Bottle.get(i);
        if (v.isList())
        {
            yarp::os::Bottle* b = v.asList();
            if (b && b->size() >= 2)
            {
                std::string key = b->get(0).asString();
                if (key == "param3")
                {
                    REQUIRE(b->get(1).asFloat64() == 20.5);
                    foundParam3 = true;
                } else if (key == "param4")
                {
                    REQUIRE(b->get(1).asString() == "value4");
                    foundParam4 = true;
                }
            }
        }
    }
    REQUIRE(foundParam3);
    REQUIRE(foundParam4);
}

TEST_CASE("YarpPropertyConverter handles deeply nested groups", "[YarpPropertyConverter]")
{
    dinrail::Parameters dinrailProp;
    dinrailProp.put("rootParam", "rootValue");

    // Create nested structure: root -> level1 -> level2 -> level3
    dinrail::Parameters& level1 = dinrailProp.addGroup("level1");
    level1.put("level1Param", 1);

    dinrail::Parameters& level2 = level1.addGroup("level2");
    level2.put("level2Param", 2);

    dinrail::Parameters& level3 = level2.addGroup("level3");
    level3.put("level3Param", 3);
    level3.put("deepValue", "veryDeep");

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    // Verify root level
    REQUIRE(yarpProp.check("rootParam"));
    REQUIRE(yarpProp.find("rootParam").asString() == "rootValue");

    // Verify level1
    REQUIRE(yarpProp.check("level1"));
    yarp::os::Bottle level1Bottle = yarpProp.findGroup("level1");
    REQUIRE(!level1Bottle.isNull());
    REQUIRE(level1Bottle.size() > 0);
    REQUIRE(level1Bottle.get(0).asString() == "level1");

    // Find level1Param and level2 group in level1
    bool foundLevel1Param = false;
    bool foundLevel2 = false;
    yarp::os::Bottle level2Bottle;
    for (size_t i = 1; i < level1Bottle.size(); ++i)
    {
        yarp::os::Value& v = level1Bottle.get(i);
        if (v.isList())
        {
            yarp::os::Bottle* b = v.asList();
            if (b && b->size() >= 2)
            {
                std::string key = b->get(0).asString();
                if (key == "level1Param")
                {
                    REQUIRE(b->get(1).asInt32() == 1);
                    foundLevel1Param = true;
                } else if (key == "level2")
                {
                    // level2 is a nested group, b is structured like (level2 prop1 prop2 ...)
                    // Keep the entire bottle - it's already in the right format
                    level2Bottle = *b;
                    foundLevel2 = true;
                }
            }
        }
    }
    REQUIRE(foundLevel1Param);
    REQUIRE(foundLevel2);
    REQUIRE(!level2Bottle.isNull());

    // Verify level2
    bool foundLevel2Param = false;
    bool foundLevel3 = false;
    yarp::os::Bottle level3Bottle;
    // level2Bottle is structured like: (level2 (level2Param 2) (level3 ...))
    // Start from index 1 to skip the group name
    for (size_t i = 1; i < level2Bottle.size(); ++i)
    {
        yarp::os::Value& v = level2Bottle.get(i);
        if (v.isList())
        {
            yarp::os::Bottle* b = v.asList();
            if (b && b->size() >= 2)
            {
                std::string key = b->get(0).asString();
                if (key == "level2Param")
                {
                    REQUIRE(b->get(1).asInt32() == 2);
                    foundLevel2Param = true;
                } else if (key == "level3")
                {
                    // level3 is also a nested group, keep the entire bottle
                    level3Bottle = *b;
                    foundLevel3 = true;
                }
            }
        }
    }
    REQUIRE(foundLevel2Param);
    REQUIRE(foundLevel3);
    REQUIRE(!level3Bottle.isNull());

    // Verify level3
    bool foundLevel3Param = false;
    bool foundDeepValue = false;
    // level3Bottle is structured like: (level3 (level3Param 3) (deepValue "veryDeep"))
    // Start from index 1 to skip the group name
    for (size_t i = 1; i < level3Bottle.size(); ++i)
    {
        yarp::os::Value& v = level3Bottle.get(i);
        if (v.isList())
        {
            yarp::os::Bottle* b = v.asList();
            if (b && b->size() >= 2)
            {
                std::string key = b->get(0).asString();
                if (key == "level3Param")
                {
                    REQUIRE(b->get(1).asInt32() == 3);
                    foundLevel3Param = true;
                } else if (key == "deepValue")
                {
                    REQUIRE(b->get(1).asString() == "veryDeep");
                    foundDeepValue = true;
                }
            }
        }
    }
    REQUIRE(foundLevel3Param);
    REQUIRE(foundDeepValue);
}

TEST_CASE("YarpPropertyConverter handles complex mixed configuration", "[YarpPropertyConverter]")
{
    // Create a complex configuration with mixed data types and nested groups
    dinrail::Parameters dinrailProp;
    dinrailProp.put("device", "complexDevice");
    dinrailProp.put("rate", 100.5);
    dinrailProp.put("enabled", 1);

    dinrail::Parameters& general = dinrailProp.addGroup("GENERAL");
    general.put("Joints", 6);
    general.put("Name", "TestRobot");

    dinrail::Parameters& limits = general.addGroup("LIMITS");
    limits.put("min", -180.0);
    limits.put("max", 180.0);

    dinrail::Parameters& network = dinrailProp.addGroup("NETWORK");
    network.put("port", 10000);
    network.put("host", "localhost");

    yarp::os::Property yarpProp = dinrail::YarpPropertyConverter::toYarpProperty(dinrailProp);

    // Verify root properties
    REQUIRE(yarpProp.check("device"));
    REQUIRE(yarpProp.find("device").asString() == "complexDevice");
    REQUIRE(yarpProp.check("rate"));
    REQUIRE(yarpProp.find("rate").asFloat64() == 100.5);
    REQUIRE(yarpProp.check("enabled"));
    REQUIRE(yarpProp.find("enabled").asInt32() == 1);

    // Verify GENERAL group exists
    REQUIRE(yarpProp.check("GENERAL"));

    // Verify NETWORK group exists
    REQUIRE(yarpProp.check("NETWORK"));
}

TEST_CASE("dinrail::Value matches YARP int-to-double compatibility",
          "[YarpPropertyConverter][ValueCompatibility]")
{
    yarp::os::Property yarpProperty;
    yarpProperty.put("intParam", 42);

    dinrail::Parameters dinrailParameters;
    dinrailParameters.put("intParam", 42);

    const yarp::os::Value& yarpValue = yarpProperty.find("intParam");
    const dinrail::Value& dinrailValue = dinrailParameters.find("intParam");

    // YARP uses isFloat64()/asFloat64() while dinrail uses isDouble()/asDouble().
    REQUIRE(yarpValue.isFloat64() == dinrailValue.isDouble());
    REQUIRE(yarpValue.asFloat64() == dinrailValue.asDouble());

    // Missing keys should still be searchable and yield null-like values.
    REQUIRE(yarpProperty.find("missing").isNull() == dinrailParameters.find("missing").isNull());
}

TEST_CASE("YarpPropertyConverter promotes mixed numeric list to vector<double>",
          "[YarpPropertyConverter]")
{
    yarp::os::Property yarpProp;
    yarp::os::Property& group = yarpProp.addGroup("LEFT_FINGERS_RETARGETING");

    yarp::os::Value* listValue = yarp::os::Value::makeList();
    yarp::os::Bottle* list = listValue->asList();
    REQUIRE(list != nullptr);
    list->addInt32(1);
    list->addFloat64(2.0);
    list->addFloat64(3.5);
    list->addFloat64(3.5);
    list->addFloat64(3.5);
    group.put("fingersScaling", listValue);

    dinrail::Parameters dinrailParams = dinrail::YarpPropertyConverter::toDinrailParameters(yarpProp);
    const dinrail::Parameters& fingersGroup = dinrailParams.findGroup("LEFT_FINGERS_RETARGETING");

    REQUIRE_FALSE(fingersGroup.isNull());
    REQUIRE(fingersGroup.check<std::vector<double>>("fingersScaling"));

    const auto& values = fingersGroup.find("fingersScaling").as<std::vector<double>>();
    REQUIRE(values.size() == 5);
    REQUIRE(values[0] == 1.0);
    REQUIRE(values[1] == 2.0);
    REQUIRE(values[2] == 3.5);
    REQUIRE(values[3] == 3.5);
    REQUIRE(values[4] == 3.5);
}
