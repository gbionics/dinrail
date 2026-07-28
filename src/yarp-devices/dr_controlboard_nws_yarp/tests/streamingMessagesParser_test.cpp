/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "StreamingMessagesParser.h"

#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Network.h>
#include <yarp/sig/Vector.h>

#include <dinrail/ControlBoardYARPProtocolSharedDefinitions.h>

#include <catch2/catch_test_macros.hpp>

#include <functional>

using namespace yarp::dev;
using namespace yarp::os;

namespace
{
// Build a streaming command message and feed it straight to the parser callback,
// bypassing the YARP streaming port. This keeps the test fast and lets us cover a
// large number of malformed payloads that used to crash StreamingMessagesParser::onRead.
void feed(StreamingMessagesParser& parser,
          const std::function<void(Bottle&, yarp::sig::Vector&)>& fill)
{
    CommandMessage msg;
    fill(msg.head, msg.body);
    parser.onRead(msg);
}

// A non-empty body is required to get past the early null-data check in onRead.
void fillBody(yarp::sig::Vector& body, size_t n)
{
    body.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
        body[i] = 0.0;
    }
}
} // namespace

TEST_CASE("dev::controlBoard_nws_yarp::StreamingMessagesParser", "[yarp::dev]")
{
    // fakeMotionControl is loaded as a YARP plugin, but no network traffic is required:
    // we call the parser callback directly, so the whole test runs in-process and
    // synchronously (much faster than round-tripping through a streaming port).
    Network::setLocalMode(true);

    PolyDriver dd_fake;
    Property p_fake;
    p_fake.put("device", "fakeMotionControl");
    Property& grp = p_fake.addGroup("GENERAL");
    grp.put("Joints", 2);
    REQUIRE(dd_fake.open(p_fake));

    StreamingMessagesParser parser;
    parser.init(&dd_fake);
    REQUIRE(parser.initialize());

    // Regression test: malformed streaming group commands used to dereference the null
    // pointer returned by Value::asList() inside onRead, segfaulting the whole process
    // (crash observed in yarp::os::Bottle::size()). A crash in any case below would take
    // down the test executable, so surviving every case is the assertion.

    SECTION("Group commands with a missing joint-selection list")
    {
        // b.get(N).asList() returns nullptr -> used to crash on jlut->size().
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_TORQUES_DIRECT_GROUP);
            head.addInt32(2);
            fillBody(body, 2);
        });
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_POSITION_DIRECT_GROUP);
            head.addInt32(2);
            fillBody(body, 2);
        });
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_VELOCITY_MOVE_GROUP);
            head.addInt32(2);
            fillBody(body, 2);
        });
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_CURRENTCONTROL_INTERFACE);
            head.addVocab32(VOCAB_CURRENT_REF_GROUP);
            head.addInt32(2);
            fillBody(body, 2);
        });
        CHECK(true);
    }

    SECTION("Group commands where the joint selection is not a list")
    {
        // The element at the list position is present but is an integer, not a Bottle
        // list: asList() also returns nullptr.
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_TORQUES_DIRECT_GROUP);
            head.addInt32(2);
            head.addInt32(42);
            fillBody(body, 2);
        });
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_POSITION_DIRECT_GROUP);
            head.addInt32(2);
            head.addInt32(42);
            fillBody(body, 2);
        });
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_VELOCITY_MOVE_GROUP);
            head.addInt32(2);
            head.addInt32(42);
            fillBody(body, 2);
        });
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_CURRENTCONTROL_INTERFACE);
            head.addVocab32(VOCAB_CURRENT_REF_GROUP);
            head.addInt32(2);
            head.addInt32(42);
            fillBody(body, 2);
        });
        CHECK(true);
    }

    SECTION("Group commands whose joint-list size does not match the joint count")
    {
        // Exercises the corrected size validation and early return (no out-of-bounds read).
        auto mismatched
            = [](int declared, int listSize, size_t bodySize, int vocab, bool currentInterface) {
                  return [=](Bottle& head, yarp::sig::Vector& body) {
                      if (currentInterface)
                      {
                          head.addVocab32(VOCAB_CURRENTCONTROL_INTERFACE);
                      }
                      head.addVocab32(vocab);
                      head.addInt32(declared);
                      Bottle& jl = head.addList();
                      for (int i = 0; i < listSize; ++i)
                      {
                          jl.addInt32(i);
                      }
                      fillBody(body, bodySize);
                  };
              };

        // list too short, list too long, and body-size mismatch
        feed(parser, mismatched(2, 1, 2, VOCAB_TORQUES_DIRECT_GROUP, false));
        feed(parser, mismatched(2, 3, 2, VOCAB_POSITION_DIRECT_GROUP, false));
        feed(parser, mismatched(2, 2, 1, VOCAB_VELOCITY_MOVE_GROUP, false));
        feed(parser, mismatched(2, 1, 2, VOCAB_CURRENT_REF_GROUP, true));
        CHECK(true);
    }

    SECTION("Group commands with a zero or negative declared joint count")
    {
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_TORQUES_DIRECT_GROUP);
            head.addInt32(-1);
            head.addList().addInt32(0);
            fillBody(body, 2);
        });
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_POSITION_DIRECT_GROUP);
            head.addInt32(0);
            head.addList();
            fillBody(body, 2);
        });
        CHECK(true);
    }

    SECTION("Impedance-all-setpoints commands with malformed payloads")
    {
        parser.setImpedanceAllSetPointsControlEmulation(true);

        // wrong sub-command vocab
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(dinrail::VOCAB_DINRAIL_IMPEDANCE_ALL_SETPOINTS);
            head.addVocab32(0);
            head.addInt32(1);
            fillBody(body, 5);
        });
        // group form (b.size()==4) with the joint list missing (not a list)
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(dinrail::VOCAB_DINRAIL_IMPEDANCE_ALL_SETPOINTS);
            head.addVocab32(dinrail::VOCAB_DINRAIL_SETPOINT);
            head.addInt32(1);
            head.addInt32(7); // should have been a list
            fillBody(body, 5);
        });
        // group form with a mismatched joint list size / payload
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(dinrail::VOCAB_DINRAIL_IMPEDANCE_ALL_SETPOINTS);
            head.addVocab32(dinrail::VOCAB_DINRAIL_SETPOINT);
            head.addInt32(2);
            head.addList().addInt32(0);
            fillBody(body, 5);
        });
        // single-joint form with wrong payload size
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(dinrail::VOCAB_DINRAIL_IMPEDANCE_ALL_SETPOINTS);
            head.addVocab32(dinrail::VOCAB_DINRAIL_SETPOINT);
            head.addInt32(0);
            fillBody(body, 3); // must be 5
        });
        // malformed header (only the vocab pair, no joint/payload)
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(dinrail::VOCAB_DINRAIL_IMPEDANCE_ALL_SETPOINTS);
            head.addVocab32(dinrail::VOCAB_DINRAIL_SETPOINT);
            fillBody(body, 1);
        });
        CHECK(true);
    }

    SECTION("Empty, truncated and unknown commands")
    {
        // completely empty message
        feed(parser, [](Bottle&, yarp::sig::Vector&) {});
        // empty head but non-empty body
        feed(parser, [](Bottle&, yarp::sig::Vector& body) { fillBody(body, 2); });
        // unknown top-level vocab
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(yarp::os::createVocab32('z', 'z', 'z', 'z'));
            fillBody(body, 2);
        });
        // interface-prefixed command with an unknown sub-command
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_CURRENTCONTROL_INTERFACE);
            head.addVocab32(yarp::os::createVocab32('z', 'z', 'z', 'z'));
            fillBody(body, 2);
        });
        // single-joint command with an empty body (early null-data return path)
        feed(parser, [](Bottle& head, yarp::sig::Vector&) {
            head.addVocab32(VOCAB_VELOCITY_MOVE);
            head.addInt32(0);
        });
        CHECK(true);
    }

    SECTION("Well-formed commands are still handled after malformed traffic")
    {
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_TORQUES_DIRECT_GROUP);
            head.addInt32(2);
            Bottle& jl = head.addList();
            jl.addInt32(0);
            jl.addInt32(1);
            fillBody(body, 2);
        });
        feed(parser, [](Bottle& head, yarp::sig::Vector& body) {
            head.addVocab32(VOCAB_POSITION_DIRECTS);
            fillBody(body, 2);
        });
        CHECK(true);
    }

    parser.reset();
    CHECK(dd_fake.close());

    Network::setLocalMode(false);
}
