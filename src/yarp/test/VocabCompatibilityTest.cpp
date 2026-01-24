// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/IControlMode.h>
#include <dinrail/Vocab.h>

#include <yarp/dev/IControlMode.h>
#include <yarp/os/Vocab.h>

TEST_CASE("YARP Vocab Compatibility", "[yarp][vocab]")
{
    SECTION("Control Mode Vocabs Match YARP")
    {
        // Verify that dinrail vocab values match YARP vocab values exactly
        CHECK(dinrail::VOCAB_CM_IDLE == VOCAB_CM_IDLE);
        CHECK(dinrail::VOCAB_CM_TORQUE == VOCAB_CM_TORQUE);
        CHECK(dinrail::VOCAB_CM_POSITION == VOCAB_CM_POSITION);
        CHECK(dinrail::VOCAB_CM_POSITION_DIRECT == VOCAB_CM_POSITION_DIRECT);
        CHECK(dinrail::VOCAB_CM_VELOCITY == VOCAB_CM_VELOCITY);
        CHECK(dinrail::VOCAB_CM_CURRENT == VOCAB_CM_CURRENT);
        CHECK(dinrail::VOCAB_CM_PWM == VOCAB_CM_PWM);
        CHECK(dinrail::VOCAB_CM_MIXED == VOCAB_CM_MIXED);
        CHECK(dinrail::VOCAB_CM_FORCE_IDLE == VOCAB_CM_FORCE_IDLE);
        CHECK(dinrail::VOCAB_CM_HW_FAULT == VOCAB_CM_HW_FAULT);
        CHECK(dinrail::VOCAB_CM_CALIBRATING == VOCAB_CM_CALIBRATING);
        CHECK(dinrail::VOCAB_CM_CALIB_DONE == VOCAB_CM_CALIB_DONE);
        CHECK(dinrail::VOCAB_CM_NOT_CONFIGURED == VOCAB_CM_NOT_CONFIGURED);
        CHECK(dinrail::VOCAB_CM_CONFIGURED == VOCAB_CM_CONFIGURED);
        CHECK(dinrail::VOCAB_CM_UNKNOWN == VOCAB_CM_UNKNOWN);
    }

    SECTION("createVocab32 produces same values as YARP")
    {
        // Test a few examples
        CHECK(dinrail::createVocab32('s','e','t') == yarp::os::createVocab32('s','e','t'));
        CHECK(dinrail::createVocab32('p','o','s') == yarp::os::createVocab32('p','o','s'));
        CHECK(dinrail::createVocab32('i','d','l') == yarp::os::createVocab32('i','d','l'));
        CHECK(dinrail::createVocab32('p','o','s','d') == yarp::os::createVocab32('p','o','s','d'));
        CHECK(dinrail::createVocab32('t','o','r','q') == yarp::os::createVocab32('t','o','r','q'));
    }

    SECTION("Vocab decode produces human-readable strings")
    {
        CHECK(dinrail::Vocab32::decode(dinrail::VOCAB_CM_IDLE) == "idl");
        CHECK(dinrail::Vocab32::decode(dinrail::VOCAB_CM_POSITION) == "pos");
        CHECK(dinrail::Vocab32::decode(dinrail::VOCAB_CM_POSITION_DIRECT) == "posd");
        CHECK(dinrail::Vocab32::decode(dinrail::VOCAB_CM_TORQUE) == "torq");
        CHECK(dinrail::Vocab32::decode(dinrail::VOCAB_CM_VELOCITY) == "vel");
    }

    SECTION("Vocab encode/decode round-trip")
    {
        std::string test1 = "set";
        CHECK(dinrail::Vocab32::decode(dinrail::Vocab32::encode(test1)) == test1);
        
        std::string test2 = "pos";
        CHECK(dinrail::Vocab32::decode(dinrail::Vocab32::encode(test2)) == test2);
        
        std::string test3 = "posd";
        CHECK(dinrail::Vocab32::decode(dinrail::Vocab32::encode(test3)) == test3);
    }
}
