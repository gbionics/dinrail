/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "DinRailControlBoardNWSRtShm.h"

#include <yarp/os/LogStream.h>

#include <chrono>
#include <cmath>
#include <cstring>
#include <numeric>

#include <poll.h>
#include <unistd.h>

using namespace yarp::dev;
using namespace yarp::os;
using namespace dinrail::rtshm;

namespace {
constexpr int kPidNumFields = 10;
constexpr int kMotorTorqueNumFields = 9;
} // namespace

DinRailControlBoardNWSRtShm::~DinRailControlBoardNWSRtShm()
{
    close();
}

// ===========================================================================
// Lifecycle
// ===========================================================================
bool DinRailControlBoardNWSRtShm::open(Searchable& config)
{
    if (!parseParams(config))
    {
        yError() << "dr_controlboard_nws_rtshm: missing mandatory 'name' parameter";
        return false;
    }

    m_socketPath = m_endpoint.empty() ? socketPathForName(m_name) : m_endpoint;
    if (!m_server.listen(m_socketPath))
    {
        yError() << "dr_controlboard_nws_rtshm: cannot create server socket at" << m_socketPath;
        return false;
    }

    m_running = true;
    m_mgmtThread = std::thread(&DinRailControlBoardNWSRtShm::managementLoop, this);
    return true;
}

bool DinRailControlBoardNWSRtShm::close()
{
    m_running = false;
    m_attached = false;

    m_jobCv.notify_all();
    m_doneCv.notify_all();

    if (m_serviceThread.joinable())
    {
        m_serviceThread.join();
    }

    m_server.close(); // unblocks accept()
    if (m_mgmtThread.joinable())
    {
        m_mgmtThread.join();
    }

    clearInterfaces();
    m_region.close();
    return true;
}

bool DinRailControlBoardNWSRtShm::attach(yarp::dev::PolyDriver* poly)
{
    if (m_attached)
    {
        return false;
    }
    if (!setDevice(poly))
    {
        return false;
    }

    m_generation = nowNs() ^ (static_cast<std::uint64_t>(::getpid()) << 40);
    if (!m_region.create(static_cast<std::uint32_t>(m_joints), m_generation, m_lock_shared_memory))
    {
        yError() << "dr_controlboard_nws_rtshm: failed to create shared memory region";
        return false;
    }

    m_stage.resize(static_cast<std::uint32_t>(m_joints));
    m_times.assign(m_joints, 0.0);
    for (auto& s : m_scratch)
    {
        s.assign(m_joints, 0.0);
    }
    for (auto& seq : m_mailboxLastSeq)
    {
        seq = 0;
    }

    m_attached = true;
    m_serviceThread = std::thread(&DinRailControlBoardNWSRtShm::serviceLoop, this);
    return true;
}

bool DinRailControlBoardNWSRtShm::detach()
{
    m_attached = false;
    m_jobCv.notify_all();
    if (m_serviceThread.joinable())
    {
        m_serviceThread.join();
    }
    clearInterfaces();
    return true;
}

bool DinRailControlBoardNWSRtShm::setDevice(yarp::dev::DeviceDriver* driver)
{
    driver->view(iJointFault);
    driver->view(iPidControl);
    driver->view(iPositionControl);
    driver->view(iPositionDirect);
    driver->view(iVelocityControl);
    driver->view(iEncodersTimed);
    driver->view(iMotor);
    driver->view(iMotorEncoders);
    driver->view(iAmplifierControl);
    driver->view(iControlLimits);
    driver->view(iControlCalibration);
    driver->view(iTorqueControl);
    driver->view(iImpedanceControl);
    driver->view(iImpedanceAllSetPointsControl);
    driver->view(iControlMode);
    driver->view(iAxisInfo);
    driver->view(iPreciselyTimed);
    driver->view(iInteractionMode);
    driver->view(iRemoteVariables);
    driver->view(iPWMControl);
    driver->view(iCurrentControl);

    int tmp_axes = 0;
    if (iAxisInfo)
    {
        if (!iAxisInfo->getAxes(&tmp_axes))
        {
            return false;
        }
    } else if (iEncodersTimed)
    {
        if (!iEncodersTimed->getAxes(&tmp_axes))
        {
            return false;
        }
    } else if (iPositionControl)
    {
        if (!iPositionControl->getAxes(&tmp_axes))
        {
            return false;
        }
    } else if (iVelocityControl)
    {
        if (!iVelocityControl->getAxes(&tmp_axes))
        {
            return false;
        }
    }

    if (tmp_axes <= 0)
    {
        yError() << "dr_controlboard_nws_rtshm: attached device has invalid number of joints"
                 << tmp_axes;
        return false;
    }
    m_joints = static_cast<std::size_t>(tmp_axes);
    return true;
}

void DinRailControlBoardNWSRtShm::clearInterfaces()
{
    iJointFault = nullptr;
    iPidControl = nullptr;
    iPositionControl = nullptr;
    iPositionDirect = nullptr;
    iVelocityControl = nullptr;
    iEncodersTimed = nullptr;
    iMotor = nullptr;
    iMotorEncoders = nullptr;
    iAmplifierControl = nullptr;
    iControlLimits = nullptr;
    iControlCalibration = nullptr;
    iTorqueControl = nullptr;
    iImpedanceControl = nullptr;
    iImpedanceAllSetPointsControl = nullptr;
    iControlMode = nullptr;
    iAxisInfo = nullptr;
    iPreciselyTimed = nullptr;
    iInteractionMode = nullptr;
    iRemoteVariables = nullptr;
    iPWMControl = nullptr;
    iCurrentControl = nullptr;
}

// ===========================================================================
// Management thread: accept a client, handshake, forward RPC to the service thread.
// ===========================================================================
void DinRailControlBoardNWSRtShm::managementLoop()
{
    while (m_running)
    {
        struct pollfd pfd{};
        pfd.fd = m_server.fd();
        pfd.events = POLLIN;
        int pr = ::poll(&pfd, 1, 200);
        if (!m_running)
        {
            break;
        }
        if (pr <= 0)
        {
            continue; // timeout or interrupted: re-check m_running
        }
        UnixConnection conn = m_server.accept();
        if (!conn.valid())
        {
            if (!m_running)
            {
                break;
            }
            continue;
        }
        serveClient(conn);
    }
}

void DinRailControlBoardNWSRtShm::serveClient(UnixConnection& conn)
{
    conn.setRecvTimeout(200);

    std::vector<std::uint8_t> buf(kMaxRpcPacket);
    while (m_running)
    {
        RpcPacketHeader hdr{};
        std::uint32_t n = 0;
        if (!conn.recvPacket(hdr, buf.data(), n))
        {
            if (!m_running || conn.peerClosed())
            {
                return;
            }
            continue; // recv timeout: keep waiting
        }

        if (hdr.opcode == kOpHello)
        {
            // Wait until a subdevice is attached and the region exists.
            while (m_running && !m_attached)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            if (!m_running)
            {
                return;
            }
            std::vector<std::uint8_t> resp(64);
            PayloadWriter w(resp.data(), resp.size());
            w.putU32(static_cast<std::uint32_t>(m_joints));
            w.putU64(m_generation);
            w.putU64(m_region.layout().regionBytes);
            RpcPacketHeader rhdr{kRpcMagic,
                                 kRpcMajor,
                                 kRpcMinor,
                                 kOpHello,
                                 kRpcFlagOk,
                                 hdr.request_id,
                                 static_cast<std::uint32_t>(w.size()),
                                 0};
            conn.sendPacketWithFd(rhdr, resp.data(), m_region.fd());
            continue;
        }

        // Hand the request over to the service thread which owns the wrapped device.
        RpcJob job;
        job.opcode = hdr.opcode;
        job.req.assign(buf.data(), buf.data() + n);

        {
            std::unique_lock<std::mutex> lk(m_jobMutex);
            m_jobs.push(&job);
            m_jobCv.notify_one();
            m_doneCv.wait(lk, [&] { return job.done || !m_running || !m_attached; });
        }

        RpcPacketHeader rhdr{kRpcMagic,
                             kRpcMajor,
                             kRpcMinor,
                             hdr.opcode,
                             job.ok ? kRpcFlagOk : 0u,
                             hdr.request_id,
                             job.respBytes,
                             0};
        if (!conn.sendPacket(rhdr, job.resp.data()))
        {
            return;
        }
    }
}

// ===========================================================================
// Service thread: single owner of the wrapped device.
// ===========================================================================
void DinRailControlBoardNWSRtShm::serviceLoop()
{
    using clock = std::chrono::steady_clock;
    auto period = std::chrono::duration_cast<clock::duration>(
        std::chrono::duration<double>(m_period > 0 ? m_period : 0.01));
    auto next = clock::now();

    while (m_running && m_attached)
    {
        std::unique_lock<std::mutex> lk(m_jobMutex);
        m_jobCv.wait_until(lk, next, [&] { return !m_jobs.empty() || !m_running || !m_attached; });

        std::queue<RpcJob*> local;
        std::swap(local, m_jobs);
        lk.unlock();

        while (!local.empty())
        {
            RpcJob* job = local.front();
            local.pop();
            dispatchRpc(*job);
            {
                std::lock_guard<std::mutex> jlk(m_jobMutex);
                job->done = true;
            }
            m_doneCv.notify_all();
        }

        if (!m_running || !m_attached)
        {
            break;
        }

        auto now = clock::now();
        if (now >= next)
        {
            runCycle();
            do
            {
                next += period;
            } while (next <= now);
        }
    }

    // Drain any waiting RPC so the socket thread does not block on shutdown.
    std::lock_guard<std::mutex> lk(m_jobMutex);
    while (!m_jobs.empty())
    {
        m_jobs.front()->done = true;
        m_jobs.pop();
    }
    m_doneCv.notify_all();
}

void DinRailControlBoardNWSRtShm::runCycle()
{
    applyMailboxes();
    populateAndPublishState();
    Heartbeat* hb = serverHeartbeat(m_region.base(), m_region.layout());
    storeRelease(hb->time_ns, nowNs());
    storeRelease(hb->seq, loadRelaxed(hb->seq) + 1);
    Diagnostics* diag = diagnostics(m_region.base(), m_region.layout());
    diag->cycles++;
}

void DinRailControlBoardNWSRtShm::applyMailboxes()
{
    const Layout& l = m_region.layout();
    void* base = m_region.base();
    std::uint32_t axis = static_cast<std::uint32_t>(m_joints);

    // Position direct
    {
        double* arr[1] = {m_scratch[0].data()};
        if (readMailbox(base, l, kMailPositionDirect, axis, m_mailboxLastSeq[kMailPositionDirect],
                        arr, 1)
            && iPositionDirect)
        {
            iPositionDirect->setPositions(m_scratch[0].data());
        }
    }
    // Velocity
    {
        double* arr[1] = {m_scratch[1].data()};
        if (readMailbox(base, l, kMailVelocity, axis, m_mailboxLastSeq[kMailVelocity], arr, 1)
            && iVelocityControl)
        {
            iVelocityControl->velocityMove(m_scratch[1].data());
        }
    }
    // Torque
    {
        double* arr[1] = {m_scratch[2].data()};
        if (readMailbox(base, l, kMailTorque, axis, m_mailboxLastSeq[kMailTorque], arr, 1)
            && iTorqueControl)
        {
            iTorqueControl->setRefTorques(m_scratch[2].data());
        }
    }
    // Current
    {
        double* arr[1] = {m_scratch[3].data()};
        if (readMailbox(base, l, kMailCurrent, axis, m_mailboxLastSeq[kMailCurrent], arr, 1)
            && iCurrentControl)
        {
            iCurrentControl->setRefCurrents(m_scratch[3].data());
        }
    }
    // PWM
    {
        double* arr[1] = {m_scratch[4].data()};
        if (readMailbox(base, l, kMailPwm, axis, m_mailboxLastSeq[kMailPwm], arr, 1) && iPWMControl)
        {
            iPWMControl->setRefDutyCycles(m_scratch[4].data());
        }
    }
    // Impedance-all (best effort; may be emulated by the underlying device)
    {
        static thread_local std::vector<double> imp[5];
        for (auto& v : imp)
        {
            if (v.size() != m_joints)
            {
                v.assign(m_joints, 0.0);
            }
        }
        double* arr[5] = {imp[0].data(), imp[1].data(), imp[2].data(), imp[3].data(), imp[4].data()};
        if (readMailbox(base, l, kMailImpedanceAll, axis, m_mailboxLastSeq[kMailImpedanceAll], arr,
                        5)
            && iImpedanceAllSetPointsControl)
        {
            std::span<const double> sPos(imp[0]);
            std::span<const double> sVel(imp[1]);
            std::span<const double> sTrq(imp[2]);
            std::span<const double> sStf(imp[3]);
            std::span<const double> sDmp(imp[4]);
            dinrail::VectorProxy<const double> pos(sPos);
            dinrail::VectorProxy<const double> vel(sVel);
            dinrail::VectorProxy<const double> trq(sTrq);
            dinrail::VectorProxy<const double> stf(sStf);
            dinrail::VectorProxy<const double> dmp(sDmp);
            iImpedanceAllSetPointsControl->setSetPoints(pos, vel, trq, stf, dmp);
        }
    }
}

void DinRailControlBoardNWSRtShm::populateAndPublishState()
{
    std::uint32_t axis = static_cast<std::uint32_t>(m_joints);
    std::uint64_t valid = 0;

    if (iEncodersTimed)
    {
        if (iEncodersTimed->getEncodersTimed(m_stage.doubles[kFieldJointPosition].data(),
                                             m_times.data()))
        {
            valid |= stateFieldBit(kFieldJointPosition);
        }
        if (iEncodersTimed->getEncoderSpeeds(m_stage.doubles[kFieldJointVelocity].data()))
        {
            valid |= stateFieldBit(kFieldJointVelocity);
        }
        if (iEncodersTimed->getEncoderAccelerations(m_stage.doubles[kFieldJointAcceleration].data()))
        {
            valid |= stateFieldBit(kFieldJointAcceleration);
        }
    }

    if (iMotorEncoders)
    {
        if (iMotorEncoders->getMotorEncoders(m_stage.doubles[kFieldMotorPosition].data()))
        {
            valid |= stateFieldBit(kFieldMotorPosition);
        }
        if (iMotorEncoders->getMotorEncoderSpeeds(m_stage.doubles[kFieldMotorVelocity].data()))
        {
            valid |= stateFieldBit(kFieldMotorVelocity);
        }
        if (iMotorEncoders->getMotorEncoderAccelerations(
                m_stage.doubles[kFieldMotorAcceleration].data()))
        {
            valid |= stateFieldBit(kFieldMotorAcceleration);
        }
    }

    if (iMotor && iMotor->getTemperatures(m_stage.doubles[kFieldTemperature].data()))
    {
        valid |= stateFieldBit(kFieldTemperature);
    }
    if (iTorqueControl && iTorqueControl->getTorques(m_stage.doubles[kFieldTorque].data()))
    {
        valid |= stateFieldBit(kFieldTorque);
    }
    if (iPWMControl && iPWMControl->getDutyCycles(m_stage.doubles[kFieldPwmDutycycle].data()))
    {
        valid |= stateFieldBit(kFieldPwmDutycycle);
    }
    if (iCurrentControl && iCurrentControl->getCurrents(m_stage.doubles[kFieldCurrent].data()))
    {
        valid |= stateFieldBit(kFieldCurrent);
    } else if (iAmplifierControl
               && iAmplifierControl->getCurrents(m_stage.doubles[kFieldCurrent].data()))
    {
        valid |= stateFieldBit(kFieldCurrent);
    }
    if (iControlMode && iControlMode->getControlModes(m_stage.ints[0].data()))
    {
        valid |= stateFieldBit(kFieldControlMode);
    }
    if (iInteractionMode
        && iInteractionMode->getInteractionModes(
            reinterpret_cast<yarp::dev::InteractionModeEnum*>(m_stage.ints[1].data())))
    {
        valid |= stateFieldBit(kFieldInteractionMode);
    }

    double avgTime = 0.0;
    if (axis > 0)
    {
        avgTime = std::accumulate(m_times.begin(), m_times.end(), 0.0) / axis;
    }

    m_stage.axisCount = axis;
    m_stage.validFields = valid;
    m_stage.sampleTimeNs = static_cast<std::uint64_t>(avgTime * 1e9);
    m_stage.publishTimeNs = nowNs();

    publishState(m_region.base(), m_region.layout(), m_stage);

    Diagnostics* diag = diagnostics(m_region.base(), m_region.layout());
    diag->state_publications++;
}

// ===========================================================================
// RPC dispatch (executed only in the service thread).
// ===========================================================================
void DinRailControlBoardNWSRtShm::dispatchRpc(RpcJob& job)
{
    job.resp.assign(kMaxRpcPacket, 0);
    PayloadReader r(job.req.data(), job.req.size());
    PayloadWriter w(job.resp.data(), job.resp.size());
    bool ok = false;

    const std::uint32_t axis = static_cast<std::uint32_t>(m_joints);
    std::vector<std::int32_t> ji(m_joints > 0 ? m_joints : 1);
    std::vector<double> da(m_joints > 0 ? m_joints : 1);
    std::vector<double> db(m_joints > 0 ? m_joints : 1);

    switch (job.opcode)
    {
    case kOpGetAxes:
        w.putI32(static_cast<std::int32_t>(m_joints));
        ok = true;
        break;
    case kOpGetAxisName:
    {
        std::int32_t j;
        std::string name;
        if (r.getI32(j) && iAxisInfo && iAxisInfo->getAxisName(j, name))
        {
            w.putString(name);
            ok = true;
        }
        break;
    }
    case kOpGetJointType:
    {
        std::int32_t j;
        yarp::dev::JointTypeEnum t;
        if (r.getI32(j) && iAxisInfo && iAxisInfo->getJointType(j, t))
        {
            w.putI32(static_cast<std::int32_t>(t));
            ok = true;
        }
        break;
    }
    case kOpResetEncoder:
    {
        std::int32_t j;
        ok = r.getI32(j) && iEncodersTimed && iEncodersTimed->resetEncoder(j);
        break;
    }
    case kOpResetEncoders:
        ok = iEncodersTimed && iEncodersTimed->resetEncoders();
        break;
    case kOpSetEncoder:
    {
        std::int32_t j;
        double v;
        ok = r.getI32(j) && r.getF64(v) && iEncodersTimed && iEncodersTimed->setEncoder(j, v);
        break;
    }
    case kOpSetEncoders:
    {
        std::uint32_t n;
        ok = r.getF64Array(da.data(), axis, n) && iEncodersTimed
             && iEncodersTimed->setEncoders(da.data());
        break;
    }
    case kOpResetMotorEncoder:
    {
        std::int32_t j;
        ok = r.getI32(j) && iMotorEncoders && iMotorEncoders->resetMotorEncoder(j);
        break;
    }
    case kOpResetMotorEncoders:
        ok = iMotorEncoders && iMotorEncoders->resetMotorEncoders();
        break;
    case kOpSetMotorEncoder:
    {
        std::int32_t j;
        double v;
        ok = r.getI32(j) && r.getF64(v) && iMotorEncoders && iMotorEncoders->setMotorEncoder(j, v);
        break;
    }
    case kOpSetMotorEncoders:
    {
        std::uint32_t n;
        ok = r.getF64Array(da.data(), axis, n) && iMotorEncoders
             && iMotorEncoders->setMotorEncoders(da.data());
        break;
    }
    case kOpSetMotorEncoderCountsPerRevolution:
    {
        std::int32_t m;
        double v;
        ok = r.getI32(m) && r.getF64(v) && iMotorEncoders
             && iMotorEncoders->setMotorEncoderCountsPerRevolution(m, v);
        break;
    }
    case kOpGetMotorEncoderCountsPerRevolution:
    {
        std::int32_t m;
        double v = 0;
        if (r.getI32(m) && iMotorEncoders
            && iMotorEncoders->getMotorEncoderCountsPerRevolution(m, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetNumberOfMotorEncoders:
    {
        int n = 0;
        if (iMotorEncoders && iMotorEncoders->getNumberOfMotorEncoders(&n))
        {
            w.putI32(n);
            ok = true;
        }
        break;
    }
    case kOpGetNumberOfMotors:
    {
        int n = 0;
        if (iMotor && iMotor->getNumberOfMotors(&n))
        {
            w.putI32(n);
            ok = true;
        }
        break;
    }
    case kOpGetTemperatureLimit:
    {
        std::int32_t m;
        double v = 0;
        if (r.getI32(m) && iMotor && iMotor->getTemperatureLimit(m, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpSetTemperatureLimit:
    {
        std::int32_t m;
        double v;
        ok = r.getI32(m) && r.getF64(v) && iMotor && iMotor->setTemperatureLimit(m, v);
        break;
    }
    case kOpGetGearboxRatio:
    {
        std::int32_t m;
        double v = 0;
        if (r.getI32(m) && iMotor && iMotor->getGearboxRatio(m, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpSetGearboxRatio:
    {
        std::int32_t m;
        double v;
        ok = r.getI32(m) && r.getF64(v) && iMotor && iMotor->setGearboxRatio(m, v);
        break;
    }
    case kOpPositionMoveSingle:
    {
        std::int32_t j;
        double v;
        ok = r.getI32(j) && r.getF64(v) && iPositionControl && iPositionControl->positionMove(j, v);
        break;
    }
    case kOpPositionMoveAll:
    {
        std::uint32_t n;
        ok = r.getF64Array(da.data(), axis, n) && iPositionControl
             && iPositionControl->positionMove(da.data());
        break;
    }
    case kOpPositionMoveGroup:
    {
        std::uint32_t nj, nv;
        ok = r.getI32Array(ji.data(), axis, nj) && r.getF64Array(da.data(), axis, nv)
             && iPositionControl
             && iPositionControl->positionMove(static_cast<int>(nj), ji.data(), da.data());
        break;
    }
    case kOpRelativeMoveSingle:
    {
        std::int32_t j;
        double v;
        ok = r.getI32(j) && r.getF64(v) && iPositionControl && iPositionControl->relativeMove(j, v);
        break;
    }
    case kOpRelativeMoveAll:
    {
        std::uint32_t n;
        ok = r.getF64Array(da.data(), axis, n) && iPositionControl
             && iPositionControl->relativeMove(da.data());
        break;
    }
    case kOpRelativeMoveGroup:
    {
        std::uint32_t nj, nv;
        ok = r.getI32Array(ji.data(), axis, nj) && r.getF64Array(da.data(), axis, nv)
             && iPositionControl
             && iPositionControl->relativeMove(static_cast<int>(nj), ji.data(), da.data());
        break;
    }
    case kOpCheckMotionDoneSingle:
    {
        std::int32_t j;
        bool f = false;
        if (r.getI32(j) && iPositionControl && iPositionControl->checkMotionDone(j, &f))
        {
            w.putBool(f);
            ok = true;
        }
        break;
    }
    case kOpCheckMotionDoneAll:
    {
        bool f = false;
        if (iPositionControl && iPositionControl->checkMotionDone(&f))
        {
            w.putBool(f);
            ok = true;
        }
        break;
    }
    case kOpCheckMotionDoneGroup:
    {
        std::uint32_t nj;
        bool f = false;
        if (r.getI32Array(ji.data(), axis, nj) && iPositionControl
            && iPositionControl->checkMotionDone(static_cast<int>(nj), ji.data(), &f))
        {
            w.putBool(f);
            ok = true;
        }
        break;
    }
    case kOpSetRefSpeedSingle:
    {
        std::int32_t j;
        double v;
        ok = r.getI32(j) && r.getF64(v) && iPositionControl && iPositionControl->setRefSpeed(j, v);
        break;
    }
    case kOpSetRefSpeedsAll:
    {
        std::uint32_t n;
        ok = r.getF64Array(da.data(), axis, n) && iPositionControl
             && iPositionControl->setRefSpeeds(da.data());
        break;
    }
    case kOpSetRefSpeedsGroup:
    {
        std::uint32_t nj, nv;
        ok = r.getI32Array(ji.data(), axis, nj) && r.getF64Array(da.data(), axis, nv)
             && iPositionControl
             && iPositionControl->setRefSpeeds(static_cast<int>(nj), ji.data(), da.data());
        break;
    }
    case kOpSetRefAccelerationSingle:
    {
        std::int32_t j;
        double v;
        ok = r.getI32(j) && r.getF64(v) && iPositionControl
             && iPositionControl->setRefAcceleration(j, v);
        break;
    }
    case kOpSetRefAccelerationsAll:
    {
        std::uint32_t n;
        ok = r.getF64Array(da.data(), axis, n) && iPositionControl
             && iPositionControl->setRefAccelerations(da.data());
        break;
    }
    case kOpSetRefAccelerationsGroup:
    {
        std::uint32_t nj, nv;
        ok = r.getI32Array(ji.data(), axis, nj) && r.getF64Array(da.data(), axis, nv)
             && iPositionControl
             && iPositionControl->setRefAccelerations(static_cast<int>(nj), ji.data(), da.data());
        break;
    }
    case kOpGetRefSpeedSingle:
    {
        std::int32_t j;
        double v = 0;
        if (r.getI32(j) && iPositionControl && iPositionControl->getRefSpeed(j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetRefSpeedsAll:
        if (iPositionControl && iPositionControl->getRefSpeeds(da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    case kOpGetRefSpeedsGroup:
    {
        std::uint32_t nj;
        if (r.getI32Array(ji.data(), axis, nj) && iPositionControl
            && iPositionControl->getRefSpeeds(static_cast<int>(nj), ji.data(), da.data()))
        {
            w.putF64Array(da.data(), nj);
            ok = true;
        }
        break;
    }
    case kOpGetRefAccelerationSingle:
    {
        std::int32_t j;
        double v = 0;
        if (r.getI32(j) && iPositionControl && iPositionControl->getRefAcceleration(j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetRefAccelerationsAll:
        if (iPositionControl && iPositionControl->getRefAccelerations(da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    case kOpGetRefAccelerationsGroup:
    {
        std::uint32_t nj;
        if (r.getI32Array(ji.data(), axis, nj) && iPositionControl
            && iPositionControl->getRefAccelerations(static_cast<int>(nj), ji.data(), da.data()))
        {
            w.putF64Array(da.data(), nj);
            ok = true;
        }
        break;
    }
    case kOpGetTargetPositionSingle:
    {
        std::int32_t j;
        double v = 0;
        if (r.getI32(j) && iPositionControl && iPositionControl->getTargetPosition(j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetTargetPositionsAll:
        if (iPositionControl && iPositionControl->getTargetPositions(da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    case kOpGetTargetPositionsGroup:
    {
        std::uint32_t nj;
        if (r.getI32Array(ji.data(), axis, nj) && iPositionControl
            && iPositionControl->getTargetPositions(static_cast<int>(nj), ji.data(), da.data()))
        {
            w.putF64Array(da.data(), nj);
            ok = true;
        }
        break;
    }
    case kOpStopSingle:
    {
        std::int32_t j;
        ok = r.getI32(j) && iPositionControl && iPositionControl->stop(j);
        break;
    }
    case kOpStopAll:
        ok = iPositionControl && iPositionControl->stop();
        break;
    case kOpStopGroup:
    {
        std::uint32_t nj;
        ok = r.getI32Array(ji.data(), axis, nj) && iPositionControl
             && iPositionControl->stop(static_cast<int>(nj), ji.data());
        break;
    }
    case kOpSetControlModeSingle:
    {
        std::int32_t j, m;
        ok = r.getI32(j) && r.getI32(m) && iControlMode && iControlMode->setControlMode(j, m);
        break;
    }
    case kOpSetControlModesAll:
    {
        std::uint32_t n;
        if (r.getI32Array(ji.data(), axis, n) && iControlMode)
        {
            ok = iControlMode->setControlModes(ji.data());
        }
        break;
    }
    case kOpSetControlModesGroup:
    {
        std::uint32_t nj, nm;
        std::vector<std::int32_t> modes(m_joints > 0 ? m_joints : 1);
        if (r.getI32Array(ji.data(), axis, nj) && r.getI32Array(modes.data(), axis, nm)
            && iControlMode)
        {
            ok = iControlMode->setControlModes(static_cast<int>(nj), ji.data(), modes.data());
        }
        break;
    }
    case kOpSetInteractionModeSingle:
    {
        std::int32_t j, m;
        ok = r.getI32(j) && r.getI32(m) && iInteractionMode
             && iInteractionMode->setInteractionMode(
                 j, static_cast<yarp::dev::InteractionModeEnum>(m));
        break;
    }
    case kOpSetInteractionModesAll:
    {
        std::uint32_t n;
        if (r.getU32(n) && n == axis && iInteractionMode)
        {
            std::vector<yarp::dev::InteractionModeEnum> modes(n);
            bool okr = true;
            for (std::uint32_t i = 0; i < n; ++i)
            {
                std::int32_t m;
                okr = okr && r.getI32(m);
                modes[i] = static_cast<yarp::dev::InteractionModeEnum>(m);
            }
            ok = okr && iInteractionMode->setInteractionModes(modes.data());
        }
        break;
    }
    case kOpSetInteractionModesGroup:
    {
        std::uint32_t nj, nm;
        if (r.getI32Array(ji.data(), axis, nj) && r.getU32(nm) && nm == nj && iInteractionMode)
        {
            std::vector<yarp::dev::InteractionModeEnum> modes(nm);
            bool okr = true;
            for (std::uint32_t i = 0; i < nm; ++i)
            {
                std::int32_t m;
                okr = okr && r.getI32(m);
                modes[i] = static_cast<yarp::dev::InteractionModeEnum>(m);
            }
            ok = okr
                 && iInteractionMode->setInteractionModes(static_cast<int>(nj), ji.data(),
                                                          modes.data());
        }
        break;
    }
    case kOpGetRefVelocitySingle:
    {
        std::int32_t j;
        double v = 0;
        if (r.getI32(j) && iVelocityControl && iVelocityControl->getRefVelocity(j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetRefVelocitiesAll:
        if (iVelocityControl && iVelocityControl->getRefVelocities(da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    case kOpGetRefVelocitiesGroup:
    {
        std::uint32_t nj;
        if (r.getI32Array(ji.data(), axis, nj) && iVelocityControl
            && iVelocityControl->getRefVelocities(static_cast<int>(nj), ji.data(), da.data()))
        {
            w.putF64Array(da.data(), nj);
            ok = true;
        }
        break;
    }
    case kOpGetRefTorqueSingle:
    {
        std::int32_t j;
        double v = 0;
        if (r.getI32(j) && iTorqueControl && iTorqueControl->getRefTorque(j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetRefTorquesAll:
        if (iTorqueControl && iTorqueControl->getRefTorques(da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    case kOpGetTorqueRange:
    {
        std::int32_t j;
        double lo = 0, hi = 0;
        if (r.getI32(j) && iTorqueControl && iTorqueControl->getTorqueRange(j, &lo, &hi))
        {
            w.putF64(lo);
            w.putF64(hi);
            ok = true;
        }
        break;
    }
    case kOpGetTorqueRanges:
        if (iTorqueControl && iTorqueControl->getTorqueRanges(da.data(), db.data()))
        {
            w.putF64Array(da.data(), axis);
            w.putF64Array(db.data(), axis);
            ok = true;
        }
        break;
    case kOpGetMotorTorqueParams:
    {
        std::int32_t j;
        yarp::dev::MotorTorqueParameters p;
        if (r.getI32(j) && iTorqueControl && iTorqueControl->getMotorTorqueParams(j, &p))
        {
            double a[kMotorTorqueNumFields] = {p.bemf,       p.bemf_scale, p.ktau,
                                               p.ktau_scale, p.viscousPos, p.viscousNeg,
                                               p.coulombPos, p.coulombNeg, p.velocityThres};
            for (double d : a)
            {
                w.putF64(d);
            }
            ok = true;
        }
        break;
    }
    case kOpSetMotorTorqueParams:
    {
        std::int32_t j;
        double a[kMotorTorqueNumFields];
        bool okr = r.getI32(j);
        for (double& d : a)
        {
            okr = okr && r.getF64(d);
        }
        if (okr && iTorqueControl)
        {
            yarp::dev::MotorTorqueParameters p;
            p.bemf = a[0];
            p.bemf_scale = a[1];
            p.ktau = a[2];
            p.ktau_scale = a[3];
            p.viscousPos = a[4];
            p.viscousNeg = a[5];
            p.coulombPos = a[6];
            p.coulombNeg = a[7];
            p.velocityThres = a[8];
            ok = iTorqueControl->setMotorTorqueParams(j, p);
        }
        break;
    }
    case kOpGetRefDutyCycleSingle:
    {
        std::int32_t j;
        double v = 0;
        if (r.getI32(j) && iPWMControl && iPWMControl->getRefDutyCycle(j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetRefDutyCyclesAll:
        if (iPWMControl && iPWMControl->getRefDutyCycles(da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    case kOpGetRefCurrentSingle:
    {
        std::int32_t j;
        double v = 0;
        if (r.getI32(j) && iCurrentControl && iCurrentControl->getRefCurrent(j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetRefCurrentsAll:
        if (iCurrentControl && iCurrentControl->getRefCurrents(da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    case kOpGetCurrentRange:
    {
        std::int32_t j;
        double lo = 0, hi = 0;
        if (r.getI32(j) && iCurrentControl && iCurrentControl->getCurrentRange(j, &lo, &hi))
        {
            w.putF64(lo);
            w.putF64(hi);
            ok = true;
        }
        break;
    }
    case kOpGetCurrentRanges:
        if (iCurrentControl && iCurrentControl->getCurrentRanges(da.data(), db.data()))
        {
            w.putF64Array(da.data(), axis);
            w.putF64Array(db.data(), axis);
            ok = true;
        }
        break;
    case kOpSetLimits:
    {
        std::int32_t j;
        double lo, hi;
        ok = r.getI32(j) && r.getF64(lo) && r.getF64(hi) && iControlLimits
             && iControlLimits->setLimits(j, lo, hi);
        break;
    }
    case kOpGetLimits:
    {
        std::int32_t j;
        double lo = 0, hi = 0;
        if (r.getI32(j) && iControlLimits && iControlLimits->getLimits(j, &lo, &hi))
        {
            w.putF64(lo);
            w.putF64(hi);
            ok = true;
        }
        break;
    }
    case kOpSetVelLimits:
    {
        std::int32_t j;
        double lo, hi;
        ok = r.getI32(j) && r.getF64(lo) && r.getF64(hi) && iControlLimits
             && iControlLimits->setVelLimits(j, lo, hi);
        break;
    }
    case kOpGetVelLimits:
    {
        std::int32_t j;
        double lo = 0, hi = 0;
        if (r.getI32(j) && iControlLimits && iControlLimits->getVelLimits(j, &lo, &hi))
        {
            w.putF64(lo);
            w.putF64(hi);
            ok = true;
        }
        break;
    }
    case kOpSetImpedance:
    {
        std::int32_t j;
        double s, d;
        ok = r.getI32(j) && r.getF64(s) && r.getF64(d) && iImpedanceControl
             && iImpedanceControl->setImpedance(j, s, d);
        break;
    }
    case kOpGetImpedance:
    {
        std::int32_t j;
        double s = 0, d = 0;
        if (r.getI32(j) && iImpedanceControl && iImpedanceControl->getImpedance(j, &s, &d))
        {
            w.putF64(s);
            w.putF64(d);
            ok = true;
        }
        break;
    }
    case kOpSetImpedanceOffset:
    {
        std::int32_t j;
        double o;
        ok = r.getI32(j) && r.getF64(o) && iImpedanceControl
             && iImpedanceControl->setImpedanceOffset(j, o);
        break;
    }
    case kOpGetImpedanceOffset:
    {
        std::int32_t j;
        double o = 0;
        if (r.getI32(j) && iImpedanceControl && iImpedanceControl->getImpedanceOffset(j, &o))
        {
            w.putF64(o);
            ok = true;
        }
        break;
    }
    case kOpGetCurrentImpedanceLimit:
    {
        std::int32_t j;
        double a = 0, b = 0, c = 0, d = 0;
        if (r.getI32(j) && iImpedanceControl
            && iImpedanceControl->getCurrentImpedanceLimit(j, &a, &b, &c, &d))
        {
            w.putF64(a);
            w.putF64(b);
            w.putF64(c);
            w.putF64(d);
            ok = true;
        }
        break;
    }
    case kOpSetPid:
    {
        std::int32_t type, j;
        double a[kPidNumFields];
        bool okr = r.getI32(type) && r.getI32(j);
        for (double& d : a)
        {
            okr = okr && r.getF64(d);
        }
        if (okr && iPidControl)
        {
            yarp::dev::Pid pid;
            pid.kp = a[0];
            pid.kd = a[1];
            pid.ki = a[2];
            pid.max_int = a[3];
            pid.max_output = a[4];
            pid.scale = a[5];
            pid.offset = a[6];
            pid.stiction_up_val = a[7];
            pid.stiction_down_val = a[8];
            pid.kff = a[9];
            ok = iPidControl->setPid(static_cast<yarp::dev::PidControlTypeEnum>(type), j, pid);
        }
        break;
    }
    case kOpSetPids:
    {
        std::int32_t type;
        std::uint32_t n;
        bool okr = r.getI32(type) && r.getU32(n) && n == axis;
        if (okr && iPidControl)
        {
            std::vector<yarp::dev::Pid> pids(n);
            for (std::uint32_t i = 0; i < n && okr; ++i)
            {
                double a[kPidNumFields];
                for (double& d : a)
                {
                    okr = okr && r.getF64(d);
                }
                pids[i].kp = a[0];
                pids[i].kd = a[1];
                pids[i].ki = a[2];
                pids[i].max_int = a[3];
                pids[i].max_output = a[4];
                pids[i].scale = a[5];
                pids[i].offset = a[6];
                pids[i].stiction_up_val = a[7];
                pids[i].stiction_down_val = a[8];
                pids[i].kff = a[9];
            }
            ok = okr
                 && iPidControl->setPids(static_cast<yarp::dev::PidControlTypeEnum>(type),
                                         pids.data());
        }
        break;
    }
    case kOpSetPidReference:
    {
        std::int32_t type, j;
        double v;
        ok = r.getI32(type) && r.getI32(j) && r.getF64(v) && iPidControl
             && iPidControl->setPidReference(static_cast<yarp::dev::PidControlTypeEnum>(type), j, v);
        break;
    }
    case kOpSetPidReferences:
    {
        std::int32_t type;
        std::uint32_t n;
        ok = r.getI32(type) && r.getF64Array(da.data(), axis, n) && iPidControl
             && iPidControl->setPidReferences(static_cast<yarp::dev::PidControlTypeEnum>(type),
                                              da.data());
        break;
    }
    case kOpSetPidErrorLimit:
    {
        std::int32_t type, j;
        double v;
        ok = r.getI32(type) && r.getI32(j) && r.getF64(v) && iPidControl
             && iPidControl->setPidErrorLimit(static_cast<yarp::dev::PidControlTypeEnum>(type), j,
                                              v);
        break;
    }
    case kOpSetPidErrorLimits:
    {
        std::int32_t type;
        std::uint32_t n;
        ok = r.getI32(type) && r.getF64Array(da.data(), axis, n) && iPidControl
             && iPidControl->setPidErrorLimits(static_cast<yarp::dev::PidControlTypeEnum>(type),
                                               da.data());
        break;
    }
    case kOpGetPidError:
    {
        std::int32_t type, j;
        double v = 0;
        if (r.getI32(type) && r.getI32(j) && iPidControl
            && iPidControl->getPidError(static_cast<yarp::dev::PidControlTypeEnum>(type), j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetPidErrors:
    {
        std::int32_t type;
        if (r.getI32(type) && iPidControl
            && iPidControl->getPidErrors(static_cast<yarp::dev::PidControlTypeEnum>(type),
                                         da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    }
    case kOpGetPid:
    {
        std::int32_t type, j;
        yarp::dev::Pid pid;
        if (r.getI32(type) && r.getI32(j) && iPidControl
            && iPidControl->getPid(static_cast<yarp::dev::PidControlTypeEnum>(type), j, &pid))
        {
            double a[kPidNumFields] = {pid.kp,     pid.kd,           pid.ki,
                                       pid.max_int, pid.max_output,   pid.scale,
                                       pid.offset,  pid.stiction_up_val, pid.stiction_down_val,
                                       pid.kff};
            for (double d : a)
            {
                w.putF64(d);
            }
            ok = true;
        }
        break;
    }
    case kOpGetPids:
    {
        std::int32_t type;
        if (r.getI32(type) && iPidControl)
        {
            std::vector<yarp::dev::Pid> pids(m_joints);
            if (iPidControl->getPids(static_cast<yarp::dev::PidControlTypeEnum>(type), pids.data()))
            {
                w.putU32(axis);
                for (std::uint32_t i = 0; i < axis; ++i)
                {
                    const yarp::dev::Pid& p = pids[i];
                    double a[kPidNumFields] = {p.kp,     p.kd,           p.ki,
                                               p.max_int, p.max_output,   p.scale,
                                               p.offset,  p.stiction_up_val, p.stiction_down_val,
                                               p.kff};
                    for (double d : a)
                    {
                        w.putF64(d);
                    }
                }
                ok = true;
            }
        }
        break;
    }
    case kOpGetPidReference:
    {
        std::int32_t type, j;
        double v = 0;
        if (r.getI32(type) && r.getI32(j) && iPidControl
            && iPidControl->getPidReference(static_cast<yarp::dev::PidControlTypeEnum>(type), j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetPidReferences:
    {
        std::int32_t type;
        if (r.getI32(type) && iPidControl
            && iPidControl->getPidReferences(static_cast<yarp::dev::PidControlTypeEnum>(type),
                                             da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    }
    case kOpGetPidErrorLimit:
    {
        std::int32_t type, j;
        double v = 0;
        if (r.getI32(type) && r.getI32(j) && iPidControl
            && iPidControl->getPidErrorLimit(static_cast<yarp::dev::PidControlTypeEnum>(type), j,
                                             &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetPidErrorLimits:
    {
        std::int32_t type;
        if (r.getI32(type) && iPidControl
            && iPidControl->getPidErrorLimits(static_cast<yarp::dev::PidControlTypeEnum>(type),
                                              da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    }
    case kOpResetPid:
    {
        std::int32_t type, j;
        ok = r.getI32(type) && r.getI32(j) && iPidControl
             && iPidControl->resetPid(static_cast<yarp::dev::PidControlTypeEnum>(type), j);
        break;
    }
    case kOpDisablePid:
    {
        std::int32_t type, j;
        ok = r.getI32(type) && r.getI32(j) && iPidControl
             && iPidControl->disablePid(static_cast<yarp::dev::PidControlTypeEnum>(type), j);
        break;
    }
    case kOpEnablePid:
    {
        std::int32_t type, j;
        ok = r.getI32(type) && r.getI32(j) && iPidControl
             && iPidControl->enablePid(static_cast<yarp::dev::PidControlTypeEnum>(type), j);
        break;
    }
    case kOpIsPidEnabled:
    {
        std::int32_t type, j;
        bool e = false;
        if (r.getI32(type) && r.getI32(j) && iPidControl
            && iPidControl->isPidEnabled(static_cast<yarp::dev::PidControlTypeEnum>(type), j, &e))
        {
            w.putBool(e);
            ok = true;
        }
        break;
    }
    case kOpGetPidOutput:
    {
        std::int32_t type, j;
        double v = 0;
        if (r.getI32(type) && r.getI32(j) && iPidControl
            && iPidControl->getPidOutput(static_cast<yarp::dev::PidControlTypeEnum>(type), j, &v))
        {
            w.putF64(v);
            ok = true;
        }
        break;
    }
    case kOpGetPidOutputs:
    {
        std::int32_t type;
        if (r.getI32(type) && iPidControl
            && iPidControl->getPidOutputs(static_cast<yarp::dev::PidControlTypeEnum>(type),
                                          da.data()))
        {
            w.putF64Array(da.data(), axis);
            ok = true;
        }
        break;
    }
    case kOpSetPidOffset:
    {
        std::int32_t type, j;
        double v;
        ok = r.getI32(type) && r.getI32(j) && r.getF64(v) && iPidControl
             && iPidControl->setPidOffset(static_cast<yarp::dev::PidControlTypeEnum>(type), j, v);
        break;
    }
    case kOpGetLastJointFault:
    {
        std::int32_t j;
        int fault = 0;
        std::string message;
        if (r.getI32(j) && iJointFault && iJointFault->getLastJointFault(j, fault, message))
        {
            w.putI32(fault);
            w.putString(message);
            ok = true;
        }
        break;
    }
    default:
        ok = false;
        break;
    }

    job.ok = ok;
    job.respBytes = static_cast<std::uint32_t>(w.size());
}
