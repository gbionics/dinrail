/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_CONTROLBOARD_NWS_RTSHM_H
#define DINRAIL_CONTROLBOARD_NWS_RTSHM_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/WrapperSingle.h>

#include <yarp/dev/IAmplifierControl.h>
#include <yarp/dev/IAxisInfo.h>
#include <yarp/dev/IControlCalibration.h>
#include <yarp/dev/IControlLimits.h>
#include <yarp/dev/IControlMode.h>
#include <yarp/dev/ICurrentControl.h>
#include <yarp/dev/IEncodersTimed.h>
#include <yarp/dev/IImpedanceControl.h>
#include <yarp/dev/IInteractionMode.h>
#include <yarp/dev/IJointFault.h>
#include <yarp/dev/IMotor.h>
#include <yarp/dev/IMotorEncoders.h>
#include <yarp/dev/IPWMControl.h>
#include <yarp/dev/IPidControl.h>
#include <yarp/dev/IPositionControl.h>
#include <yarp/dev/IPositionDirect.h>
#include <yarp/dev/IPreciselyTimed.h>
#include <yarp/dev/IRemoteVariables.h>
#include <yarp/dev/ITorqueControl.h>
#include <yarp/dev/IVelocityControl.h>

#include <dinrail/IImpedanceAllSetPointsControl.h>

#include "DinRailControlBoardNWSRtShm_ParamsParser.h"

#include <RtShmLayout.h>
#include <RtShmProtocol.h>
#include <RtShmTransport.h>

/**
 * \brief `dr_controlboard_nws_rtshm`: real-time shared-memory server side of the control board.
 *
 * Wraps a control-board subdevice (like `dr_controlboard_nws_yarp`) but publishes its state and
 * receives commands through the RT-safe shared-memory transport described in
 * docs/rtshm_design_doc.md. A single service thread owns all access to the wrapped device: it
 * publishes the state snapshot, applies the high-rate setpoint mailboxes and executes management
 * RPC requests handed over by the socket thread.
 */
class DinRailControlBoardNWSRtShm : public yarp::dev::DeviceDriver,
                                    public yarp::dev::WrapperSingle,
                                    public DinRailControlBoardNWSRtShm_ParamsParser
{
public:
    DinRailControlBoardNWSRtShm() = default;
    ~DinRailControlBoardNWSRtShm() override;
    DinRailControlBoardNWSRtShm(const DinRailControlBoardNWSRtShm&) = delete;
    DinRailControlBoardNWSRtShm& operator=(const DinRailControlBoardNWSRtShm&) = delete;

    // DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;

    // WrapperSingle
    bool attach(yarp::dev::PolyDriver* poly) override;
    bool detach() override;

private:
    struct RpcJob
    {
        std::uint32_t opcode{0};
        std::vector<std::uint8_t> req;
        std::vector<std::uint8_t> resp;
        std::uint32_t respBytes{0};
        bool ok{false};
        bool done{false};
    };

    bool setDevice(yarp::dev::DeviceDriver* driver);
    void clearInterfaces();

    void managementLoop();
    void serviceLoop();
    void serveClient(dinrail::rtshm::UnixConnection& conn);
    void runCycle();
    void applyMailboxes();
    void populateAndPublishState();
    void dispatchRpc(RpcJob& job);

    // Transport
    dinrail::rtshm::SharedRegion m_region;
    dinrail::rtshm::UnixSocketServer m_server;
    std::string m_socketPath;
    std::uint64_t m_generation{0};

    // Threads and coordination
    std::thread m_serviceThread;
    std::thread m_mgmtThread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_attached{false};

    std::mutex m_jobMutex;
    std::condition_variable m_jobCv;
    std::condition_variable m_doneCv;
    std::queue<RpcJob*> m_jobs;

    // State staging and mailbox bookkeeping
    dinrail::rtshm::StateSnapshot m_stage;
    std::vector<double> m_times;
    std::uint64_t m_mailboxLastSeq[dinrail::rtshm::kNumMailboxFamilies]{};
    std::vector<double> m_scratch[6];

    std::size_t m_joints{0};

    // Wrapped interfaces
    yarp::dev::IJointFault* iJointFault{nullptr};
    yarp::dev::IPidControl* iPidControl{nullptr};
    yarp::dev::IPositionControl* iPositionControl{nullptr};
    yarp::dev::IPositionDirect* iPositionDirect{nullptr};
    yarp::dev::IVelocityControl* iVelocityControl{nullptr};
    yarp::dev::IEncodersTimed* iEncodersTimed{nullptr};
    yarp::dev::IMotor* iMotor{nullptr};
    yarp::dev::IMotorEncoders* iMotorEncoders{nullptr};
    yarp::dev::IAmplifierControl* iAmplifierControl{nullptr};
    yarp::dev::IControlLimits* iControlLimits{nullptr};
    yarp::dev::IControlCalibration* iControlCalibration{nullptr};
    yarp::dev::ITorqueControl* iTorqueControl{nullptr};
    yarp::dev::IImpedanceControl* iImpedanceControl{nullptr};
    dinrail::IImpedanceAllSetPointsControl* iImpedanceAllSetPointsControl{nullptr};
    yarp::dev::IControlMode* iControlMode{nullptr};
    yarp::dev::IAxisInfo* iAxisInfo{nullptr};
    yarp::dev::IPreciselyTimed* iPreciselyTimed{nullptr};
    yarp::dev::IInteractionMode* iInteractionMode{nullptr};
    yarp::dev::IRemoteVariables* iRemoteVariables{nullptr};
    yarp::dev::IPWMControl* iPWMControl{nullptr};
    yarp::dev::ICurrentControl* iCurrentControl{nullptr};
};

#endif // DINRAIL_CONTROLBOARD_NWS_RTSHM_H
