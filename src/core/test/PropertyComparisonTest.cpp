// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Property.h>
#include <yarp/os/Property.h>

TEST_CASE("dinrail::Property and yarp::os::Property basic put/check behavior", "[Property]")
{
    dinrail::Property dinrailProp;
    yarp::os::Property yarpProp;

    SECTION("put and check string values")
    {
        dinrailProp.put("device", "fakeMotionControl");
        yarpProp.put("device", "fakeMotionControl");

        REQUIRE(dinrailProp.check("device") == yarpProp.check("device"));
        REQUIRE(dinrailProp.check("device") == true);

        REQUIRE(dinrailProp.check("nonexistent") == yarpProp.check("nonexistent"));
        REQUIRE(dinrailProp.check("nonexistent") == false);
    }

    SECTION("put and check integer values")
    {
        dinrailProp.put("joints", 6);
        yarpProp.put("joints", 6);

        REQUIRE(dinrailProp.check("joints") == yarpProp.check("joints"));
        REQUIRE(dinrailProp.check("joints") == true);
    }

    SECTION("put and check double values")
    {
        dinrailProp.put("rate", 0.01);
        yarpProp.put("rate", 0.01);

        REQUIRE(dinrailProp.check("rate") == yarpProp.check("rate"));
        REQUIRE(dinrailProp.check("rate") == true);
    }

    SECTION("clear removes all entries")
    {
        dinrailProp.put("key1", "value1");
        dinrailProp.put("key2", 42);
        yarpProp.put("key1", "value1");
        yarpProp.put("key2", 42);

        REQUIRE(dinrailProp.check("key1") == true);
        REQUIRE(yarpProp.check("key1") == true);

        dinrailProp.clear();
        yarpProp.clear();

        REQUIRE(dinrailProp.check("key1") == yarpProp.check("key1"));
        REQUIRE(dinrailProp.check("key1") == false);
        REQUIRE(dinrailProp.check("key2") == yarpProp.check("key2"));
        REQUIRE(dinrailProp.check("key2") == false);
    }
}

TEST_CASE("dinrail::Property and yarp::os::Property addGroup behavior", "[Property]")
{
    dinrail::Property dinrailProp;
    yarp::os::Property yarpProp;

    SECTION("addGroup creates a nested group")
    {
        dinrail::Property& dinrailGroup = dinrailProp.addGroup("GENERAL");
        yarp::os::Property& yarpGroup = yarpProp.addGroup("GENERAL");

        dinrailGroup.put("Joints", 6);
        yarpGroup.put("Joints", 6);

        // Both should report the group exists
        REQUIRE(dinrailProp.check("GENERAL") == yarpProp.check("GENERAL"));
        REQUIRE(dinrailProp.check("GENERAL") == true);

        // The nested group should have the value
        REQUIRE(dinrailGroup.check("Joints") == yarpGroup.check("Joints"));
        REQUIRE(dinrailGroup.check("Joints") == true);
    }

    SECTION("addGroup creates new group each time (YARP behavior)")
    {
        // NOTE: In YARP, calling addGroup twice with the same key
        // creates a new group each time, clearing the previous one.
        // The old reference becomes invalid/empty.

        dinrail::Property& dinrailGroup1 = dinrailProp.addGroup("SECTION");
        dinrailGroup1.put("value1", 10);

        // Calling addGroup again clears the group and creates a new one
        dinrail::Property& dinrailGroup2 = dinrailProp.addGroup("SECTION");
        dinrailGroup2.put("value2", 20);

        // Only value2 should exist (value1 was cleared)
        REQUIRE(dinrailGroup2.check("value1") == false);
        REQUIRE(dinrailGroup2.check("value2") == true);

        // Same behavior in YARP - addGroup clears existing group
        yarp::os::Property& yarpGroup1 = yarpProp.addGroup("SECTION");
        yarpGroup1.put("value1", 10);

        yarp::os::Property& yarpGroup2 = yarpProp.addGroup("SECTION");
        yarpGroup2.put("value2", 20);

        REQUIRE(yarpGroup2.check("value1") == false);
        REQUIRE(yarpGroup2.check("value2") == true);
    }

    SECTION("clear removes groups as well")
    {
        dinrail::Property& dinrailGroup = dinrailProp.addGroup("GROUP");
        dinrailGroup.put("key", "value");

        yarp::os::Property& yarpGroup = yarpProp.addGroup("GROUP");
        yarpGroup.put("key", "value");

        REQUIRE(dinrailProp.check("GROUP") == true);
        REQUIRE(yarpProp.check("GROUP") == true);

        dinrailProp.clear();
        yarpProp.clear();

        REQUIRE(dinrailProp.check("GROUP") == yarpProp.check("GROUP"));
        REQUIRE(dinrailProp.check("GROUP") == false);
    }
}

TEST_CASE("dinrail::Property typical device configuration pattern", "[Property]")
{
    // This test verifies the typical pattern used to configure devices
    // matches between dinrail and YARP

    dinrail::Property dinrailOpts;
    yarp::os::Property yarpOpts;

    // Configure like fakeMotionControl
    dinrailOpts.put("device", "fakeMotionControl");
    yarpOpts.put("device", "fakeMotionControl");

    dinrail::Property& dinrailGeneral = dinrailOpts.addGroup("GENERAL");
    yarp::os::Property& yarpGeneral = yarpOpts.addGroup("GENERAL");

    dinrailGeneral.put("Joints", 6);
    yarpGeneral.put("Joints", 6);

    // Verify structure
    REQUIRE(dinrailOpts.check("device") == yarpOpts.check("device"));
    REQUIRE(dinrailOpts.check("GENERAL") == yarpOpts.check("GENERAL"));
    REQUIRE(dinrailGeneral.check("Joints") == yarpGeneral.check("Joints"));
}
