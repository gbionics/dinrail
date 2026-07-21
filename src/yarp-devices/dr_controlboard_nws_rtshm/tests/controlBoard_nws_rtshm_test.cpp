/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Reuses the controlBoard_nws_yarp tests against the real-time shared-memory NWS device.
 */

#include <yarp/dev/IAxisInfo.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/WrapperSingle.h>
#include <yarp/os/Network.h>

#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/yarp/dev/tests/IImpedanceAllSetPointsControlTest.h>

#include <catch2/catch_test_macros.hpp>

using namespace yarp::dev;
using namespace yarp::os;

TEST_CASE("dev::controlBoard_nws_rtshm", "[yarp::dev]")
{
    Network::setLocalMode(true);

    SECTION("Checking controlBoard_nws_rtshm device")
    {
        PolyDriver ddnws;

        {
            Property pcfg;
            pcfg.put("device", "dr_controlboard_nws_rtshm");
            pcfg.put("name", "/controlboard_a");
            REQUIRE(ddnws.open(pcfg));
        }

        {
            CHECK(ddnws.close());
        }
    }

    SECTION("Test the controlBoard_nws_rtshm device with fakeMotionControl device attached")
    {
        PolyDriver dd_fake;
        PolyDriver dd_nws;
        Property p_fake;
        Property p_nws;

        p_nws.put("device", "dr_controlboard_nws_rtshm");
        p_nws.put("name", "/controlboard_b");
        p_fake.put("device", "fakeMotionControl");
        REQUIRE(dd_fake.open(p_fake));
        REQUIRE(dd_nws.open(p_nws));

        {
            yarp::dev::WrapperSingle* ww_nws;
            dd_nws.view(ww_nws);
            REQUIRE(ww_nws);
            bool result_att = ww_nws->attach(&dd_fake);
            REQUIRE(result_att);
        }

        {
            CHECK(dd_nws.close());
            CHECK(dd_fake.close());
        }
    }

    SECTION("Test the controlBoard_nws_rtshm device with a non-attachable device")
    {
        PolyDriver dd_fake2;
        PolyDriver dd_nws;
        Property p_fake2;
        Property p_nws;

        p_nws.put("device", "dr_controlboard_nws_rtshm");
        p_nws.put("name", "/controlboard_c");
        p_fake2.put("device", "fakeSpeaker");
        REQUIRE(dd_fake2.open(p_fake2));
        REQUIRE(dd_nws.open(p_nws));

        {
            yarp::dev::WrapperSingle* ww_nws;
            dd_nws.view(ww_nws);
            REQUIRE(ww_nws);
            bool result_att = ww_nws->attach(&dd_fake2);
            REQUIRE(!result_att);
        }

        {
            CHECK(dd_nws.close());
            CHECK(dd_fake2.close());
        }
    }

    SECTION("Test the controlBoard_nws_rtshm device with a device attached which does not implement "
            "motion control interfaces")
    {
        PolyDriver dd_fake;
        PolyDriver dd_nws;
        Property p_fake;
        Property p_nws;

        p_nws.put("device", "dr_controlboard_nws_rtshm");
        p_nws.put("name", "/controlboard_d");
        p_fake.put("device", "fakeMotionControlMicro");
        REQUIRE(dd_fake.open(p_fake));
        REQUIRE(dd_nws.open(p_nws));

        {
            yarp::dev::WrapperSingle* ww_nws;
            dd_nws.view(ww_nws);
            REQUIRE(ww_nws);
            bool result_att = ww_nws->attach(&dd_fake);
            REQUIRE(result_att);
        }

        {
            CHECK(dd_nws.close());
            CHECK(dd_fake.close());
        }
    }

    SECTION("Test IImpedanceAllSetPointsControl emulation through rtshm NWS/NWC")
    {
        PolyDriver dd_fake;
        PolyDriver dd_nws;
        PolyDriver dd_nwc;

        Property p_fake;
        Property p_nws;
        Property p_nwc;

        p_fake.put("device", "fakeMotionControl");
        Property& grp = p_fake.addGroup("GENERAL");
        grp.put("Joints", 2);

        p_nws.put("device", "dr_controlboard_nws_rtshm");
        p_nws.put("name", "/controlboard_e");
        p_nws.put("emulate_impedance_all_setpoints_control", true);

        p_nwc.put("device", "dr_controlboard_nwc_rtshm");
        p_nwc.put("local", "/local_controlboard_e");
        p_nwc.put("remote", "/controlboard_e");

        REQUIRE(dd_fake.open(p_fake));
        REQUIRE(dd_nws.open(p_nws));

        {
            yarp::dev::WrapperSingle* ww_nws{nullptr};
            dd_nws.view(ww_nws);
            REQUIRE(ww_nws != nullptr);
            REQUIRE(ww_nws->attach(&dd_fake));
        }

        REQUIRE(dd_nwc.open(p_nwc));

        yarp::dev::IAxisInfo* iinfo{nullptr};
        dinrail::IImpedanceAllSetPointsControl* iimpAll{nullptr};
        dd_nwc.view(iinfo);
        dd_nwc.view(iimpAll);
        REQUIRE(iinfo != nullptr);
        REQUIRE(iimpAll != nullptr);

        yarp::dev::tests::exec_iImpedanceAllSetPointsControl_test_1(iimpAll, iinfo);

        CHECK(dd_nwc.close());
        CHECK(dd_nws.close());
        CHECK(dd_fake.close());
    }

    Network::setLocalMode(false);
}
