/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/yarp/dev/tests/IAxisInfoTest.h>
#include <dinrail/yarp/dev/tests/IControlLimitsTest.h>
#include <dinrail/yarp/dev/tests/IControlModeTest.h>
#include <dinrail/yarp/dev/tests/ICurrentControlTest.h>
#include <dinrail/yarp/dev/tests/IEncodersTimedTest.h>
#include <dinrail/yarp/dev/tests/IImpedanceAllSetPointsControlTest.h>
#include <dinrail/yarp/dev/tests/IImpedanceControlTest.h>
#include <dinrail/yarp/dev/tests/IInteractionModeTest.h>
#include <dinrail/yarp/dev/tests/IJointFaultTest.h>
#include <dinrail/yarp/dev/tests/IMotorEncodersTest.h>
#include <dinrail/yarp/dev/tests/IMotorTest.h>
#include <dinrail/yarp/dev/tests/IPWMControlTest.h>
#include <dinrail/yarp/dev/tests/IPidControlTest.h>
#include <dinrail/yarp/dev/tests/IPositionControlTest.h>
#include <dinrail/yarp/dev/tests/IRemoteCalibratorTest.h>
#include <dinrail/yarp/dev/tests/ITorqueControlTest.h>
#include <dinrail/yarp/dev/tests/IVelocityControlTest.h>
#include <yarp/dev/IAxisInfo.h>
#include <yarp/dev/IControlLimits.h>
#include <yarp/dev/IControlMode.h>
#include <yarp/dev/ICurrentControl.h>
#include <yarp/dev/IEncodersTimed.h>
#include <yarp/dev/IImpedanceControl.h>
#include <yarp/dev/IInteractionMode.h>
#include <yarp/dev/IMotor.h>
#include <yarp/dev/IMotorEncoders.h>
#include <yarp/dev/IPWMControl.h>
#include <yarp/dev/IPidControl.h>
#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/IPositionDirect.h>
#include <yarp/dev/IRemoteCalibrator.h>
#include <yarp/dev/ITorqueControl.h>
#include <yarp/dev/IVelocityControl.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/WrapperSingle.h>
#include <yarp/os/Network.h>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <thread>
#include <vector>

using namespace yarp::dev;
using namespace yarp::os;

TEST_CASE("dev::DinRailControlBoardNWCYarpTest", "[yarp::dev]")
{
    // YARP_REQUIRE_PLUGIN("fakeMotionControl", "device");
    // YARP_REQUIRE_PLUGIN("dr_controlboard_nws_yarp", "device");
    // YARP_REQUIRE_PLUGIN("dr_controlboard_nwc_yarp", "device");

    Network::setLocalMode(true);

    SECTION("Checking remote_controlboard device (using fakeMotionControl)")
    {
        PolyDriver ddmc;
        PolyDriver ddnws;
        PolyDriver ddnwc;

        IPositionControl* ipos = nullptr;
        IVelocityControl* ivel = nullptr;
        ITorqueControl* itrq = nullptr;
        IAxisInfo* iinfo = nullptr;
        IEncodersTimed* ienc = nullptr;
        IControlMode* icmd = nullptr;
        IInteractionMode* iint = nullptr;
        IMotor* imot = nullptr;
        IMotorEncoders* imotenc = nullptr;
        IPidControl* ipid = nullptr;
        IPWMControl* ipwm = nullptr;
        ICurrentControl* icurr = nullptr;
        IJointFault* ifault = nullptr;
        IControlLimits* ilims = nullptr;
        IImpedanceControl* iimp = nullptr;
        dinrail::IImpedanceAllSetPointsControl* iimpAll = nullptr;
        // IRemoteCalibrator* iremotecalib = nullptr;

        ////////"Checking opening fakeMotionControl and controlBoard_nws_yarp polydrivers"
        {
            Property p_cfg;
            p_cfg.put("device", "fakeMotionControl");
            Property& grp = p_cfg.addGroup("GENERAL");
            grp.put("Joints", 2);
            REQUIRE(ddmc.open(p_cfg));
        }
        {
            Property p_cfg;
            p_cfg.put("device", "dr_controlboard_nws_yarp");
            p_cfg.put("name", "/controlboardserver");
            p_cfg.put("emulate_impedance_all_setpoints_control", true);
            REQUIRE(ddnws.open(p_cfg));
        }

        // attach nws and fake
        {
            yarp::dev::WrapperSingle* ww_nws = nullptr;
            ddnws.view(ww_nws);
            REQUIRE(ww_nws);
            bool result_att = ww_nws->attach(&ddmc);
            REQUIRE(result_att);
        }

        // open the nwc
        {
            Property p_cfg;
            p_cfg.put("device", "dr_controlboard_nwc_yarp");
            p_cfg.put("local", "/local_controlboard");
            p_cfg.put("remote", "/controlboardserver");
            REQUIRE(ddnwc.open(p_cfg));
        }

        // test
        ddnwc.view(ipos);
        REQUIRE(ipos);
        ddnwc.view(ivel);
        REQUIRE(ivel);
        ddnwc.view(itrq);
        REQUIRE(itrq);
        ddnwc.view(iinfo);
        REQUIRE(iinfo);
        ddnwc.view(ienc);
        REQUIRE(ienc);
        ddnwc.view(icmd);
        REQUIRE(icmd);
        ddnwc.view(iint);
        REQUIRE(iint);
        ddnwc.view(imot);
        REQUIRE(imot);
        ddnwc.view(imotenc);
        REQUIRE(imotenc);
        ddnwc.view(ipid);
        REQUIRE(ipid);
        ddnwc.view(ipwm);
        REQUIRE(ipwm);
        ddnwc.view(icurr);
        REQUIRE(icurr);
        ddnwc.view(ifault);
        REQUIRE(ifault);
        ddnwc.view(ilims);
        REQUIRE(ilims);
        ddnwc.view(iimp);
        REQUIRE(iimp);
        ddnwc.view(iimpAll);
        REQUIRE(iimpAll);
        REQUIRE(iimp);
        // ddnwc.view(icalib);  REQUIRE(iremotecalib);

        yarp::dev::tests::exec_iPositionControl_test_1(ipos, icmd);
        yarp::dev::tests::exec_iVelocityControl_test_1(ivel, icmd);
        yarp::dev::tests::exec_iTorqueControl_test_1(itrq, icmd);
        yarp::dev::tests::exec_iAxisInfo_test_1(iinfo);
        yarp::dev::tests::exec_iEncodersTimed_test_1(ienc);
        yarp::dev::tests::exec_iControlMode_test_1(icmd, iinfo);
        yarp::dev::tests::exec_iInteractionMode_test_1(iint, iinfo);
        yarp::dev::tests::exec_iMotor_test_1(imot);
        yarp::dev::tests::exec_iMotorEncoders_test_1(imotenc);
        yarp::dev::tests::exec_iPidControl_test_1(ipid, iinfo);
        yarp::dev::tests::exec_iPidControl_test_2(ipid);
        yarp::dev::tests::exec_iPwmControl_test_1(ipwm, icmd);
        yarp::dev::tests::exec_iCurrentControl_test_1(icurr, icmd);
        // yarp::dev::tests::exec_iRemoteCalibrator_test_1(icalib);
        yarp::dev::tests::exec_iJointFault_test_1(ifault);
        yarp::dev::tests::exec_iControlLimits_test1(ilims, iinfo);
        yarp::dev::tests::exec_iImpedanceControl_test_1(iimp);
        yarp::dev::tests::exec_iImpedanceAllSetPointsControl_test_1(iimpAll, iinfo);

        //"Close all polydrivers and check"
        {
            CHECK(ddnwc.close());
            CHECK(ddnws.close());
            CHECK(ddmc.close());
        }
    }

    SECTION("Checking remote_controlboard device (using fakeMotionControlMicro)")
    {
        PolyDriver ddmc;
        PolyDriver ddnws;
        PolyDriver ddnwc;

        IPositionControl* ipos = nullptr;
        IVelocityControl* ivel = nullptr;
        ITorqueControl* itrq = nullptr;
        IAxisInfo* iinfo = nullptr;
        IEncodersTimed* ienc = nullptr;
        IControlMode* icmd = nullptr;

        ////////"Checking opening fakeMotionControl and controlBoard_nws_yarp polydrivers"
        {
            Property p_cfg;
            p_cfg.put("device", "fakeMotionControlMicro");
            Property& grp = p_cfg.addGroup("GENERAL");
            grp.put("Joints", 2);
            REQUIRE(ddmc.open(p_cfg));
        }
        {
            Property p_cfg;
            p_cfg.put("device", "dr_controlboard_nws_yarp");
            p_cfg.put("name", "/controlboardserver");
            REQUIRE(ddnws.open(p_cfg));
        }

        // attach nws and fake
        {
            yarp::dev::WrapperSingle* ww_nws = nullptr;
            ddnws.view(ww_nws);
            REQUIRE(ww_nws);
            bool result_att = ww_nws->attach(&ddmc);
            REQUIRE(result_att);
        }

        // open the nwc
        {
            Property p_cfg;
            p_cfg.put("device", "dr_controlboard_nwc_yarp");
            p_cfg.put("local", "/local_controlboard");
            p_cfg.put("remote", "/controlboardserver");
            REQUIRE(ddnwc.open(p_cfg));
        }

        // test
        ddnwc.view(ipos);
        REQUIRE(ipos);
        ddnwc.view(ivel);
        REQUIRE(ivel);
        ddnwc.view(itrq);
        REQUIRE(itrq);
        ddnwc.view(iinfo);
        REQUIRE(iinfo);
        ddnwc.view(ienc);
        REQUIRE(ienc);
        ddnwc.view(icmd);
        REQUIRE(icmd);
        yarp::dev::tests::exec_iPositionControl_test_unimplemented_interface(ipos, icmd);
        yarp::dev::tests::exec_iVelocityControl_test_unimplemented_interface(ivel, icmd);
        yarp::dev::tests::exec_iTorqueControl_test_unimplemented_interface(itrq, icmd);
        yarp::dev::tests::exec_iAxisInfo_test_1(iinfo);
        yarp::dev::tests::exec_iEncodersTimed_test_1(ienc);

        //"Close all polydrivers and check"
        {
            CHECK(ddnwc.close());
            CHECK(ddnws.close());
            CHECK(ddmc.close());
        }
    }

    SECTION("Concurrent streaming commands must not corrupt the command buffer")
    {
        // Regression test: the NWC streaming setpoint methods share a single
        // yarp::os::PortWriterBuffer<CommandMessage>, which is not safe for concurrent
        // get()/write(). Calling them from several threads at once used to race and crash
        // while serializing the Bottle (yarp::os::impl::BottleImpl::synch during write).
        // With the command-buffer mutex in place this must run to completion; a crash would
        // take down the whole test executable.
        PolyDriver ddmc;
        PolyDriver ddnws;
        PolyDriver ddnwc;

        {
            Property p_cfg;
            p_cfg.put("device", "fakeMotionControl");
            Property& grp = p_cfg.addGroup("GENERAL");
            grp.put("Joints", 2);
            REQUIRE(ddmc.open(p_cfg));
        }
        {
            Property p_cfg;
            p_cfg.put("device", "dr_controlboard_nws_yarp");
            p_cfg.put("name", "/controlboardserver");
            REQUIRE(ddnws.open(p_cfg));
        }
        {
            yarp::dev::WrapperSingle* ww_nws = nullptr;
            ddnws.view(ww_nws);
            REQUIRE(ww_nws);
            REQUIRE(ww_nws->attach(&ddmc));
        }
        {
            Property p_cfg;
            p_cfg.put("device", "dr_controlboard_nwc_yarp");
            p_cfg.put("local", "/local_controlboard");
            p_cfg.put("remote", "/controlboardserver");
            REQUIRE(ddnwc.open(p_cfg));
        }

        IVelocityControl* ivel = nullptr;
        ITorqueControl* itrq = nullptr;
        IPositionDirect* iposdir = nullptr;
        IPWMControl* ipwm = nullptr;
        ICurrentControl* icurr = nullptr;
        dinrail::IImpedanceAllSetPointsControl* iimpAll = nullptr;
        ddnwc.view(ivel);
        ddnwc.view(itrq);
        ddnwc.view(iposdir);
        ddnwc.view(ipwm);
        ddnwc.view(icurr);
        ddnwc.view(iimpAll);
        REQUIRE(ivel);
        REQUIRE(itrq);
        REQUIRE(iposdir);
        REQUIRE(ipwm);
        REQUIRE(icurr);
        REQUIRE(iimpAll);

        constexpr int nJoints = 2;
        const int joints[nJoints] = {0, 1};

        // Each worker hammers a different family of streaming commands (both single-joint
        // and whole-part / group variants) so that many threads contend for the buffer.
        auto worker = [&](int id) {
            double values[nJoints] = {0.1 * id, 0.2 * id};
            for (int i = 0; i < 400; ++i)
            {
                switch (id % 6)
                {
                case 0:
                    ivel->velocityMove(0, values[0]);
                    ivel->velocityMove(values);
                    ivel->velocityMove(nJoints, joints, values);
                    break;
                case 1:
                    itrq->setRefTorque(1, values[1]);
                    itrq->setRefTorques(values);
                    itrq->setRefTorques(nJoints, joints, values);
                    break;
                case 2:
                    iposdir->setPosition(0, values[0]);
                    iposdir->setPositions(values);
                    iposdir->setPositions(nJoints, joints, values);
                    break;
                case 3:
                    ipwm->setRefDutyCycle(1, values[1]);
                    ipwm->setRefDutyCycles(values);
                    break;
                case 4:
                    icurr->setRefCurrent(0, values[0]);
                    icurr->setRefCurrents(values);
                    icurr->setRefCurrents(nJoints, joints, values);
                    break;
                default:
                    iimpAll->setSetPoint(0, values[0], values[1], values[0], values[1], values[0]);
                    break;
                }
            }
        };

        std::vector<std::thread> threads;
        for (int t = 0; t < 6; ++t)
        {
            threads.emplace_back(worker, t);
        }
        for (auto& th : threads)
        {
            th.join();
        }

        // Reaching this point without crashing is the assertion.
        CHECK(true);

        {
            CHECK(ddnwc.close());
            CHECK(ddnws.close());
            CHECK(ddmc.close());
        }
    }

    Network::setLocalMode(false);
}
