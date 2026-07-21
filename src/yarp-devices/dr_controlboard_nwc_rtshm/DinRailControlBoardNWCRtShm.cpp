/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "DinRailControlBoardNWCRtShm.h"

#include <yarp/os/LogStream.h>
#include <yarp/os/Time.h>

#include <array>
#include <cstring>

using namespace yarp::dev;
using namespace yarp::os;
using namespace dinrail::rtshm;

namespace {
constexpr int kPidNumFields = 10;
constexpr int kMotorTorqueNumFields = 9;

void pidToArray(const Pid& p, double* a)
{
    a[0] = p.kp;
    a[1] = p.kd;
    a[2] = p.ki;
    a[3] = p.max_int;
    a[4] = p.max_output;
    a[5] = p.scale;
    a[6] = p.offset;
    a[7] = p.stiction_up_val;
    a[8] = p.stiction_down_val;
    a[9] = p.kff;
}

void arrayToPid(const double* a, Pid& p)
{
    p.kp = a[0];
    p.kd = a[1];
    p.ki = a[2];
    p.max_int = a[3];
    p.max_output = a[4];
    p.scale = a[5];
    p.offset = a[6];
    p.stiction_up_val = a[7];
    p.stiction_down_val = a[8];
    p.kff = a[9];
}

void motorTorqueToArray(const MotorTorqueParameters& p, double* a)
{
    a[0] = p.bemf;
    a[1] = p.bemf_scale;
    a[2] = p.ktau;
    a[3] = p.ktau_scale;
    a[4] = p.viscousPos;
    a[5] = p.viscousNeg;
    a[6] = p.coulombPos;
    a[7] = p.coulombNeg;
    a[8] = p.velocityThres;
}

void arrayToMotorTorque(const double* a, MotorTorqueParameters& p)
{
    p.bemf = a[0];
    p.bemf_scale = a[1];
    p.ktau = a[2];
    p.ktau_scale = a[3];
    p.viscousPos = a[4];
    p.viscousNeg = a[5];
    p.coulombPos = a[6];
    p.coulombNeg = a[7];
    p.velocityThres = a[8];
}
} // namespace

// ===========================================================================
// Lifecycle
// ===========================================================================
bool DinRailControlBoardNWCRtShm::open(Searchable& config)
{
    if (!parseParams(config))
    {
        yError() << "dr_controlboard_nwc_rtshm: missing mandatory 'remote' parameter";
        return false;
    }

    m_stateTimeout = static_cast<double>(m_state_timeout_ms) / 1000.0;

    std::string path = m_endpoint.empty() ? socketPathForName(m_remote) : m_endpoint;
    m_conn = connectUnixSocket(path, m_connect_timeout_ms);
    if (!m_conn.valid())
    {
        yError() << "dr_controlboard_nwc_rtshm: cannot connect to server socket" << path;
        return false;
    }
    m_conn.setRecvTimeout(m_rpc_timeout_ms);

    if (!handshake())
    {
        yError() << "dr_controlboard_nwc_rtshm: handshake failed";
        return false;
    }

    m_shadowPosDirect.assign(m_axes, 0.0);
    m_shadowVelocity.assign(m_axes, 0.0);
    m_shadowTorque.assign(m_axes, 0.0);
    m_shadowCurrent.assign(m_axes, 0.0);
    m_shadowPwm.assign(m_axes, 0.0);
    m_shadowImpPos.assign(m_axes, 0.0);
    m_shadowImpVel.assign(m_axes, 0.0);
    m_shadowImpTorque.assign(m_axes, 0.0);
    m_shadowImpStiffness.assign(m_axes, 0.0);
    m_shadowImpDamping.assign(m_axes, 0.0);
    m_state.resize(m_axes);

    m_open = true;

    // Wait for the first state snapshot so that streamed getters have data.
    double deadline = Time::now() + 1.0;
    while (Time::now() < deadline)
    {
        std::lock_guard<std::mutex> lk(m_stateMutex);
        if (readState(m_region.base(), m_region.layout(), m_state) && m_state.sequence > 0)
        {
            break;
        }
        Time::delay(0.005);
    }

    return true;
}

bool DinRailControlBoardNWCRtShm::close()
{
    m_open = false;
    m_conn.close();
    m_region.close();
    return true;
}

bool DinRailControlBoardNWCRtShm::handshake()
{
    RpcPacketHeader hdr{kRpcMagic, kRpcMajor, kRpcMinor, kOpHello, 0, ++m_requestId, 0, 0};
    if (!m_conn.sendPacket(hdr, nullptr))
    {
        return false;
    }
    RpcPacketHeader rhdr{};
    std::vector<std::uint8_t> buf(kMaxRpcPacket);
    std::uint32_t n = 0;
    int fd = -1;
    if (!m_conn.recvPacketWithFd(rhdr, buf.data(), n, fd) || fd < 0)
    {
        return false;
    }
    PayloadReader r(buf.data(), n);
    std::uint32_t axes = 0;
    std::uint64_t generation = 0;
    std::uint64_t regionBytes = 0;
    if (!r.getU32(axes) || !r.getU64(generation) || !r.getU64(regionBytes))
    {
        return false;
    }
    if (!m_region.adopt(fd, axes, generation))
    {
        return false;
    }
    m_axes = axes;
    m_generation = generation;
    return true;
}

// ===========================================================================
// State snapshot helpers
// ===========================================================================
bool DinRailControlBoardNWCRtShm::refreshState()
{
    if (!m_open)
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    readState(m_region.base(), m_region.layout(), m_state);
    if (m_state.sequence == 0)
    {
        return false;
    }
    double ageNs = static_cast<double>(nowNs()) - static_cast<double>(m_state.publishTimeNs);
    return ageNs <= m_stateTimeout * 1e9;
}

bool DinRailControlBoardNWCRtShm::stateDoubleVector(std::uint32_t field, double* v)
{
    if (!refreshState())
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    if ((m_state.validFields & stateFieldBit(field)) == 0)
    {
        return false;
    }
    std::memcpy(v, m_state.doubles[field].data(), std::size_t{m_axes} * sizeof(double));
    return true;
}

bool DinRailControlBoardNWCRtShm::stateDoubleSingle(std::uint32_t field, int j, double* v)
{
    if (!refreshState())
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    if ((m_state.validFields & stateFieldBit(field)) == 0 || j < 0
        || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    *v = m_state.doubles[field][static_cast<std::size_t>(j)];
    return true;
}

bool DinRailControlBoardNWCRtShm::stateIntVector(std::uint32_t field, int* v)
{
    if (!refreshState())
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    if ((m_state.validFields & stateFieldBit(field)) == 0)
    {
        return false;
    }
    std::uint32_t idx = field - kNumStateDoubleFields;
    std::memcpy(v, m_state.ints[idx].data(), std::size_t{m_axes} * sizeof(std::int32_t));
    return true;
}

bool DinRailControlBoardNWCRtShm::stateIntSingle(std::uint32_t field, int j, int* v)
{
    if (!refreshState())
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    if ((m_state.validFields & stateFieldBit(field)) == 0 || j < 0
        || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    std::uint32_t idx = field - kNumStateDoubleFields;
    *v = m_state.ints[idx][static_cast<std::size_t>(j)];
    return true;
}

// ===========================================================================
// RPC helpers
// ===========================================================================
bool DinRailControlBoardNWCRtShm::rpc(
    std::uint32_t opcode,
    const std::function<void(PayloadWriter&)>& build,
    const std::function<bool(PayloadReader&)>& parse)
{
    if (!m_open && opcode != kOpHello)
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_rpcMutex);
    std::vector<std::uint8_t> reqBuf(kMaxRpcPacket);
    PayloadWriter w(reqBuf.data(), reqBuf.size());
    build(w);
    RpcPacketHeader hdr{kRpcMagic,
                        kRpcMajor,
                        kRpcMinor,
                        opcode,
                        0,
                        ++m_requestId,
                        static_cast<std::uint32_t>(w.size()),
                        0};
    if (!m_conn.sendPacket(hdr, reqBuf.data()))
    {
        return false;
    }
    RpcPacketHeader rhdr{};
    std::vector<std::uint8_t> respBuf(kMaxRpcPacket);
    std::uint32_t n = 0;
    if (!m_conn.recvPacket(rhdr, respBuf.data(), n))
    {
        return false;
    }
    if ((rhdr.flags & kRpcFlagOk) == 0)
    {
        return false;
    }
    PayloadReader r(respBuf.data(), n);
    return parse(r);
}

bool DinRailControlBoardNWCRtShm::rpcSimple(
    std::uint32_t opcode, const std::function<void(PayloadWriter&)>& build)
{
    return rpc(opcode, build, [](PayloadReader&) { return true; });
}

bool DinRailControlBoardNWCRtShm::rpcVoid(std::uint32_t op)
{
    return rpcSimple(op, [](PayloadWriter&) {});
}

bool DinRailControlBoardNWCRtShm::rpcVoidJoint(std::uint32_t op, int j)
{
    return rpcSimple(op, [&](PayloadWriter& w) { w.putI32(j); });
}

bool DinRailControlBoardNWCRtShm::rpcSetSingleD(std::uint32_t op, int j, double v)
{
    return rpcSimple(op, [&](PayloadWriter& w) {
        w.putI32(j);
        w.putF64(v);
    });
}

bool DinRailControlBoardNWCRtShm::rpcGetSingleD(std::uint32_t op, int j, double* v)
{
    return rpc(
        op,
        [&](PayloadWriter& w) { w.putI32(j); },
        [&](PayloadReader& r) { return r.getF64(*v); });
}

bool DinRailControlBoardNWCRtShm::rpcSetSingleDD(std::uint32_t op, int j, double a, double b)
{
    return rpcSimple(op, [&](PayloadWriter& w) {
        w.putI32(j);
        w.putF64(a);
        w.putF64(b);
    });
}

bool DinRailControlBoardNWCRtShm::rpcGetSingleDD(std::uint32_t op, int j, double* a, double* b)
{
    return rpc(
        op,
        [&](PayloadWriter& w) { w.putI32(j); },
        [&](PayloadReader& r) { return r.getF64(*a) && r.getF64(*b); });
}

bool DinRailControlBoardNWCRtShm::rpcSetArrayAll(std::uint32_t op, const double* v)
{
    return rpcSimple(op, [&](PayloadWriter& w) { w.putF64Array(v, m_axes); });
}

bool DinRailControlBoardNWCRtShm::rpcGetArrayAll(std::uint32_t op, double* v)
{
    return rpc(
        op,
        [](PayloadWriter&) {},
        [&](PayloadReader& r) {
            std::uint32_t n = 0;
            return r.getF64Array(v, m_axes, n);
        });
}

bool DinRailControlBoardNWCRtShm::rpcSetArrayGroup(std::uint32_t op,
                                                   int n,
                                                   const int* joints,
                                                   const double* v)
{
    return rpcSimple(op, [&](PayloadWriter& w) {
        w.putI32Array(joints, static_cast<std::uint32_t>(n));
        w.putF64Array(v, static_cast<std::uint32_t>(n));
    });
}

bool DinRailControlBoardNWCRtShm::rpcGetArrayGroup(std::uint32_t op,
                                                   int n,
                                                   const int* joints,
                                                   double* v)
{
    return rpc(
        op,
        [&](PayloadWriter& w) { w.putI32Array(joints, static_cast<std::uint32_t>(n)); },
        [&](PayloadReader& r) {
            std::uint32_t rn = 0;
            return r.getF64Array(v, static_cast<std::uint32_t>(n), rn);
        });
}

bool DinRailControlBoardNWCRtShm::rpcGetPairAll(std::uint32_t op, double* a, double* b)
{
    return rpc(
        op,
        [](PayloadWriter&) {},
        [&](PayloadReader& r) {
            std::uint32_t n = 0;
            return r.getF64Array(a, m_axes, n) && r.getF64Array(b, m_axes, n);
        });
}

// ===========================================================================
// Mailbox publication
// ===========================================================================
void DinRailControlBoardNWCRtShm::publishSetpointMailbox(std::uint32_t family)
{
    if (!m_open)
    {
        return;
    }
    const double* arr = nullptr;
    switch (family)
    {
    case kMailPositionDirect:
        arr = m_shadowPosDirect.data();
        break;
    case kMailVelocity:
        arr = m_shadowVelocity.data();
        break;
    case kMailTorque:
        arr = m_shadowTorque.data();
        break;
    case kMailCurrent:
        arr = m_shadowCurrent.data();
        break;
    case kMailPwm:
        arr = m_shadowPwm.data();
        break;
    default:
        return;
    }
    const double* arrays[1] = {arr};
    publishMailbox(m_region.base(), m_region.layout(), family, m_axes, arrays, 1, nowNs());
}

void DinRailControlBoardNWCRtShm::publishImpedanceMailbox()
{
    if (!m_open)
    {
        return;
    }
    const double* arrays[5] = {m_shadowImpPos.data(),
                               m_shadowImpVel.data(),
                               m_shadowImpTorque.data(),
                               m_shadowImpStiffness.data(),
                               m_shadowImpDamping.data()};
    publishMailbox(m_region.base(), m_region.layout(), kMailImpedanceAll, m_axes, arrays, 5,
                   nowNs());
}

// ===========================================================================
// getAxes / IPreciselyTimed
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getAxes(int* ax)
{
    if (!m_open)
    {
        return false;
    }
    *ax = static_cast<int>(m_axes);
    return true;
}

yarp::os::Stamp DinRailControlBoardNWCRtShm::getLastInputStamp()
{
    refreshState();
    std::lock_guard<std::mutex> lk(m_stateMutex);
    return yarp::os::Stamp(static_cast<int>(m_state.sequence),
                           static_cast<double>(m_state.publishTimeNs) / 1e9);
}

// ===========================================================================
// IEncoders (reads from state, writes over RPC)
// ===========================================================================
bool DinRailControlBoardNWCRtShm::resetEncoder(int j)
{
    return rpcVoidJoint(kOpResetEncoder, j);
}
bool DinRailControlBoardNWCRtShm::resetEncoders()
{
    return rpcVoid(kOpResetEncoders);
}
bool DinRailControlBoardNWCRtShm::setEncoder(int j, double val)
{
    return rpcSetSingleD(kOpSetEncoder, j, val);
}
bool DinRailControlBoardNWCRtShm::setEncoders(const double* vals)
{
    return rpcSetArrayAll(kOpSetEncoders, vals);
}
bool DinRailControlBoardNWCRtShm::getEncoder(int j, double* v)
{
    return stateDoubleSingle(kFieldJointPosition, j, v);
}
bool DinRailControlBoardNWCRtShm::getEncoderTimed(int j, double* v, double* t)
{
    if (!stateDoubleSingle(kFieldJointPosition, j, v))
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    *t = static_cast<double>(m_state.publishTimeNs) / 1e9;
    return true;
}
bool DinRailControlBoardNWCRtShm::getEncoders(double* encs)
{
    return stateDoubleVector(kFieldJointPosition, encs);
}
bool DinRailControlBoardNWCRtShm::getEncodersTimed(double* encs, double* ts)
{
    if (!stateDoubleVector(kFieldJointPosition, encs))
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    double t = static_cast<double>(m_state.publishTimeNs) / 1e9;
    for (std::uint32_t i = 0; i < m_axes; ++i)
    {
        ts[i] = t;
    }
    return true;
}
bool DinRailControlBoardNWCRtShm::getEncoderSpeed(int j, double* sp)
{
    return stateDoubleSingle(kFieldJointVelocity, j, sp);
}
bool DinRailControlBoardNWCRtShm::getEncoderSpeeds(double* spds)
{
    return stateDoubleVector(kFieldJointVelocity, spds);
}
bool DinRailControlBoardNWCRtShm::getEncoderAcceleration(int j, double* acc)
{
    return stateDoubleSingle(kFieldJointAcceleration, j, acc);
}
bool DinRailControlBoardNWCRtShm::getEncoderAccelerations(double* accs)
{
    return stateDoubleVector(kFieldJointAcceleration, accs);
}

// ===========================================================================
// IMotorEncoders
// ===========================================================================
bool DinRailControlBoardNWCRtShm::resetMotorEncoder(int j)
{
    return rpcVoidJoint(kOpResetMotorEncoder, j);
}
bool DinRailControlBoardNWCRtShm::resetMotorEncoders()
{
    return rpcVoid(kOpResetMotorEncoders);
}
bool DinRailControlBoardNWCRtShm::setMotorEncoder(int j, const double val)
{
    return rpcSetSingleD(kOpSetMotorEncoder, j, val);
}
bool DinRailControlBoardNWCRtShm::setMotorEncoderCountsPerRevolution(int m, const double cpr)
{
    return rpcSetSingleD(kOpSetMotorEncoderCountsPerRevolution, m, cpr);
}
bool DinRailControlBoardNWCRtShm::getMotorEncoderCountsPerRevolution(int m, double* cpr)
{
    return rpcGetSingleD(kOpGetMotorEncoderCountsPerRevolution, m, cpr);
}
bool DinRailControlBoardNWCRtShm::setMotorEncoders(const double* vals)
{
    return rpcSetArrayAll(kOpSetMotorEncoders, vals);
}
bool DinRailControlBoardNWCRtShm::getMotorEncoder(int j, double* v)
{
    return stateDoubleSingle(kFieldMotorPosition, j, v);
}
bool DinRailControlBoardNWCRtShm::getMotorEncoderTimed(int j, double* v, double* t)
{
    if (!stateDoubleSingle(kFieldMotorPosition, j, v))
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    *t = static_cast<double>(m_state.publishTimeNs) / 1e9;
    return true;
}
bool DinRailControlBoardNWCRtShm::getMotorEncoders(double* encs)
{
    return stateDoubleVector(kFieldMotorPosition, encs);
}
bool DinRailControlBoardNWCRtShm::getMotorEncodersTimed(double* encs, double* ts)
{
    if (!stateDoubleVector(kFieldMotorPosition, encs))
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    double t = static_cast<double>(m_state.publishTimeNs) / 1e9;
    for (std::uint32_t i = 0; i < m_axes; ++i)
    {
        ts[i] = t;
    }
    return true;
}
bool DinRailControlBoardNWCRtShm::getMotorEncoderSpeed(int j, double* sp)
{
    return stateDoubleSingle(kFieldMotorVelocity, j, sp);
}
bool DinRailControlBoardNWCRtShm::getMotorEncoderSpeeds(double* spds)
{
    return stateDoubleVector(kFieldMotorVelocity, spds);
}
bool DinRailControlBoardNWCRtShm::getMotorEncoderAcceleration(int j, double* acc)
{
    return stateDoubleSingle(kFieldMotorAcceleration, j, acc);
}
bool DinRailControlBoardNWCRtShm::getMotorEncoderAccelerations(double* accs)
{
    return stateDoubleVector(kFieldMotorAcceleration, accs);
}
bool DinRailControlBoardNWCRtShm::getNumberOfMotorEncoders(int* num)
{
    return rpc(
        kOpGetNumberOfMotorEncoders,
        [](PayloadWriter&) {},
        [&](PayloadReader& r) {
            std::int32_t v = 0;
            if (!r.getI32(v))
            {
                return false;
            }
            *num = v;
            return true;
        });
}

// ===========================================================================
// IMotor
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getNumberOfMotors(int* num)
{
    return rpc(
        kOpGetNumberOfMotors,
        [](PayloadWriter&) {},
        [&](PayloadReader& r) {
            std::int32_t v = 0;
            if (!r.getI32(v))
            {
                return false;
            }
            *num = v;
            return true;
        });
}
bool DinRailControlBoardNWCRtShm::getTemperature(int m, double* val)
{
    return stateDoubleSingle(kFieldTemperature, m, val);
}
bool DinRailControlBoardNWCRtShm::getTemperatures(double* vals)
{
    return stateDoubleVector(kFieldTemperature, vals);
}
bool DinRailControlBoardNWCRtShm::getTemperatureLimit(int m, double* val)
{
    return rpcGetSingleD(kOpGetTemperatureLimit, m, val);
}
bool DinRailControlBoardNWCRtShm::setTemperatureLimit(int m, const double val)
{
    return rpcSetSingleD(kOpSetTemperatureLimit, m, val);
}
bool DinRailControlBoardNWCRtShm::getGearboxRatio(int m, double* val)
{
    return rpcGetSingleD(kOpGetGearboxRatio, m, val);
}
bool DinRailControlBoardNWCRtShm::setGearboxRatio(int m, const double val)
{
    return rpcSetSingleD(kOpSetGearboxRatio, m, val);
}

// ===========================================================================
// IPositionControl
// ===========================================================================
bool DinRailControlBoardNWCRtShm::positionMove(int j, double ref)
{
    return rpcSetSingleD(kOpPositionMoveSingle, j, ref);
}
bool DinRailControlBoardNWCRtShm::positionMove(const int n_joint, const int* joints, const double* refs)
{
    return rpcSetArrayGroup(kOpPositionMoveGroup, n_joint, joints, refs);
}
bool DinRailControlBoardNWCRtShm::positionMove(const double* refs)
{
    return rpcSetArrayAll(kOpPositionMoveAll, refs);
}
bool DinRailControlBoardNWCRtShm::getTargetPosition(const int joint, double* ref)
{
    return rpcGetSingleD(kOpGetTargetPositionSingle, joint, ref);
}
bool DinRailControlBoardNWCRtShm::getTargetPositions(double* refs)
{
    return rpcGetArrayAll(kOpGetTargetPositionsAll, refs);
}
bool DinRailControlBoardNWCRtShm::getTargetPositions(const int n_joint, const int* joints, double* refs)
{
    return rpcGetArrayGroup(kOpGetTargetPositionsGroup, n_joint, joints, refs);
}
bool DinRailControlBoardNWCRtShm::relativeMove(int j, double delta)
{
    return rpcSetSingleD(kOpRelativeMoveSingle, j, delta);
}
bool DinRailControlBoardNWCRtShm::relativeMove(const int n_joint, const int* joints, const double* refs)
{
    return rpcSetArrayGroup(kOpRelativeMoveGroup, n_joint, joints, refs);
}
bool DinRailControlBoardNWCRtShm::relativeMove(const double* deltas)
{
    return rpcSetArrayAll(kOpRelativeMoveAll, deltas);
}
bool DinRailControlBoardNWCRtShm::checkMotionDone(int j, bool* flag)
{
    return rpc(
        kOpCheckMotionDoneSingle,
        [&](PayloadWriter& w) { w.putI32(j); },
        [&](PayloadReader& r) { return r.getBool(*flag); });
}
bool DinRailControlBoardNWCRtShm::checkMotionDone(const int n_joint, const int* joints, bool* flag)
{
    return rpc(
        kOpCheckMotionDoneGroup,
        [&](PayloadWriter& w) { w.putI32Array(joints, static_cast<std::uint32_t>(n_joint)); },
        [&](PayloadReader& r) { return r.getBool(*flag); });
}
bool DinRailControlBoardNWCRtShm::checkMotionDone(bool* flag)
{
    return rpc(
        kOpCheckMotionDoneAll,
        [](PayloadWriter&) {},
        [&](PayloadReader& r) { return r.getBool(*flag); });
}
bool DinRailControlBoardNWCRtShm::setRefSpeed(int j, double sp)
{
    return rpcSetSingleD(kOpSetRefSpeedSingle, j, sp);
}
bool DinRailControlBoardNWCRtShm::setRefSpeeds(const int n_joint, const int* joints, const double* spds)
{
    return rpcSetArrayGroup(kOpSetRefSpeedsGroup, n_joint, joints, spds);
}
bool DinRailControlBoardNWCRtShm::setRefSpeeds(const double* spds)
{
    return rpcSetArrayAll(kOpSetRefSpeedsAll, spds);
}
bool DinRailControlBoardNWCRtShm::setRefAcceleration(int j, double acc)
{
    return rpcSetSingleD(kOpSetRefAccelerationSingle, j, acc);
}
bool DinRailControlBoardNWCRtShm::setRefAccelerations(const int n_joint,
                                                      const int* joints,
                                                      const double* accs)
{
    return rpcSetArrayGroup(kOpSetRefAccelerationsGroup, n_joint, joints, accs);
}
bool DinRailControlBoardNWCRtShm::setRefAccelerations(const double* accs)
{
    return rpcSetArrayAll(kOpSetRefAccelerationsAll, accs);
}
bool DinRailControlBoardNWCRtShm::getRefSpeed(int j, double* ref)
{
    return rpcGetSingleD(kOpGetRefSpeedSingle, j, ref);
}
bool DinRailControlBoardNWCRtShm::getRefSpeeds(const int n_joint, const int* joints, double* spds)
{
    return rpcGetArrayGroup(kOpGetRefSpeedsGroup, n_joint, joints, spds);
}
bool DinRailControlBoardNWCRtShm::getRefSpeeds(double* spds)
{
    return rpcGetArrayAll(kOpGetRefSpeedsAll, spds);
}
bool DinRailControlBoardNWCRtShm::getRefAcceleration(int j, double* acc)
{
    return rpcGetSingleD(kOpGetRefAccelerationSingle, j, acc);
}
bool DinRailControlBoardNWCRtShm::getRefAccelerations(const int n_joint, const int* joints, double* accs)
{
    return rpcGetArrayGroup(kOpGetRefAccelerationsGroup, n_joint, joints, accs);
}
bool DinRailControlBoardNWCRtShm::getRefAccelerations(double* accs)
{
    return rpcGetArrayAll(kOpGetRefAccelerationsAll, accs);
}
bool DinRailControlBoardNWCRtShm::stop(int j)
{
    return rpcVoidJoint(kOpStopSingle, j);
}
bool DinRailControlBoardNWCRtShm::stop(const int len, const int* val1)
{
    return rpcSimple(kOpStopGroup,
                     [&](PayloadWriter& w) { w.putI32Array(val1, static_cast<std::uint32_t>(len)); });
}
bool DinRailControlBoardNWCRtShm::stop()
{
    return rpcVoid(kOpStopAll);
}

// ===========================================================================
// IJointFault
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getLastJointFault(int j, int& fault, std::string& message)
{
    return rpc(
        kOpGetLastJointFault,
        [&](PayloadWriter& w) { w.putI32(j); },
        [&](PayloadReader& r) {
            std::int32_t f = 0;
            if (!r.getI32(f))
            {
                return false;
            }
            fault = f;
            return r.getString(message);
        });
}

// ===========================================================================
// IVelocityControl
// ===========================================================================
bool DinRailControlBoardNWCRtShm::velocityMove(int j, double v)
{
    if (!m_open || j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    m_shadowVelocity[static_cast<std::size_t>(j)] = v;
    publishSetpointMailbox(kMailVelocity);
    return true;
}
bool DinRailControlBoardNWCRtShm::velocityMove(const double* v)
{
    if (!m_open)
    {
        return false;
    }
    std::memcpy(m_shadowVelocity.data(), v, std::size_t{m_axes} * sizeof(double));
    publishSetpointMailbox(kMailVelocity);
    return true;
}
bool DinRailControlBoardNWCRtShm::velocityMove(const int n_joint, const int* joints, const double* spds)
{
    if (!m_open)
    {
        return false;
    }
    for (int i = 0; i < n_joint; ++i)
    {
        if (joints[i] >= 0 && static_cast<std::uint32_t>(joints[i]) < m_axes)
        {
            m_shadowVelocity[static_cast<std::size_t>(joints[i])] = spds[i];
        }
    }
    publishSetpointMailbox(kMailVelocity);
    return true;
}
bool DinRailControlBoardNWCRtShm::getRefVelocity(const int joint, double* vel)
{
    return rpcGetSingleD(kOpGetRefVelocitySingle, joint, vel);
}
bool DinRailControlBoardNWCRtShm::getRefVelocities(double* vels)
{
    return rpcGetArrayAll(kOpGetRefVelocitiesAll, vels);
}
bool DinRailControlBoardNWCRtShm::getRefVelocities(const int n_joint, const int* joints, double* vels)
{
    return rpcGetArrayGroup(kOpGetRefVelocitiesGroup, n_joint, joints, vels);
}

// ===========================================================================
// IControlMode
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getControlMode(int j, int* mode)
{
    return stateIntSingle(kFieldControlMode, j, mode);
}
bool DinRailControlBoardNWCRtShm::getControlModes(int* modes)
{
    return stateIntVector(kFieldControlMode, modes);
}
bool DinRailControlBoardNWCRtShm::getControlModes(const int n_joint, const int* joints, int* modes)
{
    if (!refreshState())
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    if ((m_state.validFields & stateFieldBit(kFieldControlMode)) == 0)
    {
        return false;
    }
    for (int i = 0; i < n_joint; ++i)
    {
        if (joints[i] < 0 || static_cast<std::uint32_t>(joints[i]) >= m_axes)
        {
            return false;
        }
        modes[i] = m_state.ints[0][static_cast<std::size_t>(joints[i])];
    }
    return true;
}
bool DinRailControlBoardNWCRtShm::setControlMode(const int j, const int mode)
{
    return rpcSimple(kOpSetControlModeSingle, [&](PayloadWriter& w) {
        w.putI32(j);
        w.putI32(mode);
    });
}
bool DinRailControlBoardNWCRtShm::setControlModes(const int n_joint, const int* joints, int* modes)
{
    return rpcSimple(kOpSetControlModesGroup, [&](PayloadWriter& w) {
        w.putI32Array(joints, static_cast<std::uint32_t>(n_joint));
        w.putI32Array(modes, static_cast<std::uint32_t>(n_joint));
    });
}
bool DinRailControlBoardNWCRtShm::setControlModes(int* modes)
{
    return rpcSimple(kOpSetControlModesAll,
                     [&](PayloadWriter& w) { w.putI32Array(modes, m_axes); });
}

// ===========================================================================
// IPositionDirect
// ===========================================================================
bool DinRailControlBoardNWCRtShm::setPosition(int j, double ref)
{
    if (!m_open || j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    m_shadowPosDirect[static_cast<std::size_t>(j)] = ref;
    publishSetpointMailbox(kMailPositionDirect);
    return true;
}
bool DinRailControlBoardNWCRtShm::setPositions(const int n_joint, const int* joints, const double* refs)
{
    if (!m_open)
    {
        return false;
    }
    for (int i = 0; i < n_joint; ++i)
    {
        if (joints[i] >= 0 && static_cast<std::uint32_t>(joints[i]) < m_axes)
        {
            m_shadowPosDirect[static_cast<std::size_t>(joints[i])] = refs[i];
        }
    }
    publishSetpointMailbox(kMailPositionDirect);
    return true;
}
bool DinRailControlBoardNWCRtShm::setPositions(const double* refs)
{
    if (!m_open)
    {
        return false;
    }
    std::memcpy(m_shadowPosDirect.data(), refs, std::size_t{m_axes} * sizeof(double));
    publishSetpointMailbox(kMailPositionDirect);
    return true;
}
bool DinRailControlBoardNWCRtShm::getRefPosition(const int joint, double* ref)
{
    if (!m_open || joint < 0 || static_cast<std::uint32_t>(joint) >= m_axes)
    {
        return false;
    }
    *ref = m_shadowPosDirect[static_cast<std::size_t>(joint)];
    return true;
}
bool DinRailControlBoardNWCRtShm::getRefPositions(double* refs)
{
    if (!m_open)
    {
        return false;
    }
    std::memcpy(refs, m_shadowPosDirect.data(), std::size_t{m_axes} * sizeof(double));
    return true;
}
bool DinRailControlBoardNWCRtShm::getRefPositions(const int n_joint, const int* joints, double* refs)
{
    if (!m_open)
    {
        return false;
    }
    for (int i = 0; i < n_joint; ++i)
    {
        if (joints[i] < 0 || static_cast<std::uint32_t>(joints[i]) >= m_axes)
        {
            return false;
        }
        refs[i] = m_shadowPosDirect[static_cast<std::size_t>(joints[i])];
    }
    return true;
}

// ===========================================================================
// IInteractionMode
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getInteractionMode(int axis, InteractionModeEnum* mode)
{
    int tmp = 0;
    if (!stateIntSingle(kFieldInteractionMode, axis, &tmp))
    {
        return false;
    }
    *mode = static_cast<InteractionModeEnum>(tmp);
    return true;
}
bool DinRailControlBoardNWCRtShm::getInteractionModes(int n_joints, int* joints, InteractionModeEnum* modes)
{
    if (!refreshState())
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    if ((m_state.validFields & stateFieldBit(kFieldInteractionMode)) == 0)
    {
        return false;
    }
    for (int i = 0; i < n_joints; ++i)
    {
        if (joints[i] < 0 || static_cast<std::uint32_t>(joints[i]) >= m_axes)
        {
            return false;
        }
        modes[i] = static_cast<InteractionModeEnum>(m_state.ints[1][static_cast<std::size_t>(joints[i])]);
    }
    return true;
}
bool DinRailControlBoardNWCRtShm::getInteractionModes(InteractionModeEnum* modes)
{
    if (!refreshState())
    {
        return false;
    }
    std::lock_guard<std::mutex> lk(m_stateMutex);
    if ((m_state.validFields & stateFieldBit(kFieldInteractionMode)) == 0)
    {
        return false;
    }
    for (std::uint32_t i = 0; i < m_axes; ++i)
    {
        modes[i] = static_cast<InteractionModeEnum>(m_state.ints[1][i]);
    }
    return true;
}
bool DinRailControlBoardNWCRtShm::setInteractionMode(int axis, InteractionModeEnum mode)
{
    return rpcSimple(kOpSetInteractionModeSingle, [&](PayloadWriter& w) {
        w.putI32(axis);
        w.putI32(static_cast<std::int32_t>(mode));
    });
}
bool DinRailControlBoardNWCRtShm::setInteractionModes(int n_joints, int* joints, InteractionModeEnum* modes)
{
    return rpcSimple(kOpSetInteractionModesGroup, [&](PayloadWriter& w) {
        w.putI32Array(joints, static_cast<std::uint32_t>(n_joints));
        w.putU32(static_cast<std::uint32_t>(n_joints));
        for (int i = 0; i < n_joints; ++i)
        {
            w.putI32(static_cast<std::int32_t>(modes[i]));
        }
    });
}
bool DinRailControlBoardNWCRtShm::setInteractionModes(InteractionModeEnum* modes)
{
    return rpcSimple(kOpSetInteractionModesAll, [&](PayloadWriter& w) {
        w.putU32(m_axes);
        for (std::uint32_t i = 0; i < m_axes; ++i)
        {
            w.putI32(static_cast<std::int32_t>(modes[i]));
        }
    });
}

// ===========================================================================
// ITorqueControl
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getRefTorque(int j, double* t)
{
    return rpcGetSingleD(kOpGetRefTorqueSingle, j, t);
}
bool DinRailControlBoardNWCRtShm::getRefTorques(double* t)
{
    return rpcGetArrayAll(kOpGetRefTorquesAll, t);
}
bool DinRailControlBoardNWCRtShm::setRefTorques(const double* t)
{
    if (!m_open)
    {
        return false;
    }
    std::memcpy(m_shadowTorque.data(), t, std::size_t{m_axes} * sizeof(double));
    publishSetpointMailbox(kMailTorque);
    return true;
}
bool DinRailControlBoardNWCRtShm::setRefTorque(int j, double v)
{
    if (!m_open || j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    m_shadowTorque[static_cast<std::size_t>(j)] = v;
    publishSetpointMailbox(kMailTorque);
    return true;
}
bool DinRailControlBoardNWCRtShm::setRefTorques(const int n_joint, const int* joints, const double* t)
{
    if (!m_open)
    {
        return false;
    }
    for (int i = 0; i < n_joint; ++i)
    {
        if (joints[i] >= 0 && static_cast<std::uint32_t>(joints[i]) < m_axes)
        {
            m_shadowTorque[static_cast<std::size_t>(joints[i])] = t[i];
        }
    }
    publishSetpointMailbox(kMailTorque);
    return true;
}
bool DinRailControlBoardNWCRtShm::setMotorTorqueParams(int j, const MotorTorqueParameters params)
{
    return rpcSimple(kOpSetMotorTorqueParams, [&](PayloadWriter& w) {
        w.putI32(j);
        std::array<double, kMotorTorqueNumFields> a{};
        motorTorqueToArray(params, a.data());
        for (double d : a)
        {
            w.putF64(d);
        }
    });
}
bool DinRailControlBoardNWCRtShm::getMotorTorqueParams(int j, MotorTorqueParameters* params)
{
    return rpc(
        kOpGetMotorTorqueParams,
        [&](PayloadWriter& w) { w.putI32(j); },
        [&](PayloadReader& r) {
            std::array<double, kMotorTorqueNumFields> a{};
            for (double& d : a)
            {
                if (!r.getF64(d))
                {
                    return false;
                }
            }
            arrayToMotorTorque(a.data(), *params);
            return true;
        });
}
bool DinRailControlBoardNWCRtShm::getTorque(int j, double* t)
{
    return stateDoubleSingle(kFieldTorque, j, t);
}
bool DinRailControlBoardNWCRtShm::getTorques(double* t)
{
    return stateDoubleVector(kFieldTorque, t);
}
bool DinRailControlBoardNWCRtShm::getTorqueRange(int j, double* min, double* max)
{
    return rpcGetSingleDD(kOpGetTorqueRange, j, min, max);
}
bool DinRailControlBoardNWCRtShm::getTorqueRanges(double* min, double* max)
{
    return rpcGetPairAll(kOpGetTorqueRanges, min, max);
}

// ===========================================================================
// IImpedanceControl
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getImpedance(int j, double* stiffness, double* damping)
{
    return rpcGetSingleDD(kOpGetImpedance, j, stiffness, damping);
}
bool DinRailControlBoardNWCRtShm::getImpedanceOffset(int j, double* offset)
{
    return rpcGetSingleD(kOpGetImpedanceOffset, j, offset);
}
bool DinRailControlBoardNWCRtShm::setImpedance(int j, double stiffness, double damping)
{
    return rpcSetSingleDD(kOpSetImpedance, j, stiffness, damping);
}
bool DinRailControlBoardNWCRtShm::setImpedanceOffset(int j, double offset)
{
    return rpcSetSingleD(kOpSetImpedanceOffset, j, offset);
}
bool DinRailControlBoardNWCRtShm::getCurrentImpedanceLimit(
    int j, double* min_stiff, double* max_stiff, double* min_damp, double* max_damp)
{
    return rpc(
        kOpGetCurrentImpedanceLimit,
        [&](PayloadWriter& w) { w.putI32(j); },
        [&](PayloadReader& r) {
            return r.getF64(*min_stiff) && r.getF64(*max_stiff) && r.getF64(*min_damp)
                   && r.getF64(*max_damp);
        });
}

// ===========================================================================
// dinrail::IImpedanceAllSetPointsControl (local shadow + mailbox publication)
// ===========================================================================
bool DinRailControlBoardNWCRtShm::setSetPoint(
    int j, double pos, double vel, double torque, double stiffness, double damping)
{
    if (!m_open || j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    std::size_t idx = static_cast<std::size_t>(j);
    m_shadowImpPos[idx] = pos;
    m_shadowImpVel[idx] = vel;
    m_shadowImpTorque[idx] = torque;
    m_shadowImpStiffness[idx] = stiffness;
    m_shadowImpDamping[idx] = damping;
    publishImpedanceMailbox();
    return true;
}
bool DinRailControlBoardNWCRtShm::setSetPoints(
    const dinrail::VectorProxy<const int>::Ref jointIndeces,
    const dinrail::VectorProxy<const double>::Ref pos,
    const dinrail::VectorProxy<const double>::Ref vel,
    const dinrail::VectorProxy<const double>::Ref torque,
    const dinrail::VectorProxy<const double>::Ref stiffness,
    const dinrail::VectorProxy<const double>::Ref damping)
{
    if (!m_open)
    {
        return false;
    }
    const auto n = jointIndeces.size();
    if (n != pos.size() || n != vel.size() || n != torque.size() || n != stiffness.size()
        || n != damping.size())
    {
        return false;
    }
    for (std::ptrdiff_t i = 0; i < n; ++i)
    {
        int j = jointIndeces[i];
        if (j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
        {
            return false;
        }
        std::size_t idx = static_cast<std::size_t>(j);
        m_shadowImpPos[idx] = pos[i];
        m_shadowImpVel[idx] = vel[i];
        m_shadowImpTorque[idx] = torque[i];
        m_shadowImpStiffness[idx] = stiffness[i];
        m_shadowImpDamping[idx] = damping[i];
    }
    publishImpedanceMailbox();
    return true;
}
bool DinRailControlBoardNWCRtShm::setSetPoints(
    const dinrail::VectorProxy<const double>::Ref pos,
    const dinrail::VectorProxy<const double>::Ref vel,
    const dinrail::VectorProxy<const double>::Ref torque,
    const dinrail::VectorProxy<const double>::Ref stiffness,
    const dinrail::VectorProxy<const double>::Ref damping)
{
    if (!m_open)
    {
        return false;
    }
    const std::ptrdiff_t expected = static_cast<std::ptrdiff_t>(m_axes);
    if (pos.size() != expected || vel.size() != expected || torque.size() != expected
        || stiffness.size() != expected || damping.size() != expected)
    {
        return false;
    }
    std::memcpy(m_shadowImpPos.data(), pos.data(), std::size_t{m_axes} * sizeof(double));
    std::memcpy(m_shadowImpVel.data(), vel.data(), std::size_t{m_axes} * sizeof(double));
    std::memcpy(m_shadowImpTorque.data(), torque.data(), std::size_t{m_axes} * sizeof(double));
    std::memcpy(m_shadowImpStiffness.data(), stiffness.data(), std::size_t{m_axes} * sizeof(double));
    std::memcpy(m_shadowImpDamping.data(), damping.data(), std::size_t{m_axes} * sizeof(double));
    publishImpedanceMailbox();
    return true;
}
bool DinRailControlBoardNWCRtShm::getSetPoint(
    int j, double& pos, double& vel, double& torque, double& stiffness, double& damping)
{
    if (!m_open || j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    std::size_t idx = static_cast<std::size_t>(j);
    pos = m_shadowImpPos[idx];
    vel = m_shadowImpVel[idx];
    torque = m_shadowImpTorque[idx];
    stiffness = m_shadowImpStiffness[idx];
    damping = m_shadowImpDamping[idx];
    return true;
}
bool DinRailControlBoardNWCRtShm::getSetPoints(
    const dinrail::VectorProxy<const int>::Ref jointIndeces,
    dinrail::VectorProxy<double>::Ref pos,
    dinrail::VectorProxy<double>::Ref vel,
    dinrail::VectorProxy<double>::Ref torque,
    dinrail::VectorProxy<double>::Ref stiffness,
    dinrail::VectorProxy<double>::Ref damping)
{
    if (!m_open)
    {
        return false;
    }
    const auto n = jointIndeces.size();
    if (n != pos.size() || n != vel.size() || n != torque.size() || n != stiffness.size()
        || n != damping.size())
    {
        return false;
    }
    for (std::ptrdiff_t i = 0; i < n; ++i)
    {
        int j = jointIndeces[i];
        if (j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
        {
            return false;
        }
        std::size_t idx = static_cast<std::size_t>(j);
        pos[i] = m_shadowImpPos[idx];
        vel[i] = m_shadowImpVel[idx];
        torque[i] = m_shadowImpTorque[idx];
        stiffness[i] = m_shadowImpStiffness[idx];
        damping[i] = m_shadowImpDamping[idx];
    }
    return true;
}
bool DinRailControlBoardNWCRtShm::getSetPoints(dinrail::VectorProxy<double>::Ref pos,
                                               dinrail::VectorProxy<double>::Ref vel,
                                               dinrail::VectorProxy<double>::Ref torque,
                                               dinrail::VectorProxy<double>::Ref stiffness,
                                               dinrail::VectorProxy<double>::Ref damping)
{
    if (!m_open)
    {
        return false;
    }
    const std::ptrdiff_t expected = static_cast<std::ptrdiff_t>(m_axes);
    if (pos.size() != expected || vel.size() != expected || torque.size() != expected
        || stiffness.size() != expected || damping.size() != expected)
    {
        return false;
    }
    for (std::uint32_t i = 0; i < m_axes; ++i)
    {
        pos[i] = m_shadowImpPos[i];
        vel[i] = m_shadowImpVel[i];
        torque[i] = m_shadowImpTorque[i];
        stiffness[i] = m_shadowImpStiffness[i];
        damping[i] = m_shadowImpDamping[i];
    }
    return true;
}

// ===========================================================================
// ICurrentControl
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getRefCurrents(double* t)
{
    return rpcGetArrayAll(kOpGetRefCurrentsAll, t);
}
bool DinRailControlBoardNWCRtShm::getRefCurrent(int j, double* t)
{
    return rpcGetSingleD(kOpGetRefCurrentSingle, j, t);
}
bool DinRailControlBoardNWCRtShm::setRefCurrents(const double* refs)
{
    if (!m_open)
    {
        return false;
    }
    std::memcpy(m_shadowCurrent.data(), refs, std::size_t{m_axes} * sizeof(double));
    publishSetpointMailbox(kMailCurrent);
    return true;
}
bool DinRailControlBoardNWCRtShm::setRefCurrent(int j, double ref)
{
    if (!m_open || j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    m_shadowCurrent[static_cast<std::size_t>(j)] = ref;
    publishSetpointMailbox(kMailCurrent);
    return true;
}
bool DinRailControlBoardNWCRtShm::setRefCurrents(const int n_joint, const int* joints, const double* refs)
{
    if (!m_open)
    {
        return false;
    }
    for (int i = 0; i < n_joint; ++i)
    {
        if (joints[i] >= 0 && static_cast<std::uint32_t>(joints[i]) < m_axes)
        {
            m_shadowCurrent[static_cast<std::size_t>(joints[i])] = refs[i];
        }
    }
    publishSetpointMailbox(kMailCurrent);
    return true;
}
bool DinRailControlBoardNWCRtShm::getCurrents(double* vals)
{
    return stateDoubleVector(kFieldCurrent, vals);
}
bool DinRailControlBoardNWCRtShm::getCurrent(int j, double* val)
{
    return stateDoubleSingle(kFieldCurrent, j, val);
}
bool DinRailControlBoardNWCRtShm::getCurrentRange(int j, double* min, double* max)
{
    return rpcGetSingleDD(kOpGetCurrentRange, j, min, max);
}
bool DinRailControlBoardNWCRtShm::getCurrentRanges(double* min, double* max)
{
    return rpcGetPairAll(kOpGetCurrentRanges, min, max);
}

// ===========================================================================
// IPWMControl
// ===========================================================================
bool DinRailControlBoardNWCRtShm::setRefDutyCycle(int j, double v)
{
    if (!m_open || j < 0 || static_cast<std::uint32_t>(j) >= m_axes)
    {
        return false;
    }
    m_shadowPwm[static_cast<std::size_t>(j)] = v;
    publishSetpointMailbox(kMailPwm);
    return true;
}
bool DinRailControlBoardNWCRtShm::setRefDutyCycles(const double* v)
{
    if (!m_open)
    {
        return false;
    }
    std::memcpy(m_shadowPwm.data(), v, std::size_t{m_axes} * sizeof(double));
    publishSetpointMailbox(kMailPwm);
    return true;
}
bool DinRailControlBoardNWCRtShm::getRefDutyCycle(int j, double* ref)
{
    return rpcGetSingleD(kOpGetRefDutyCycleSingle, j, ref);
}
bool DinRailControlBoardNWCRtShm::getRefDutyCycles(double* refs)
{
    return rpcGetArrayAll(kOpGetRefDutyCyclesAll, refs);
}
bool DinRailControlBoardNWCRtShm::getDutyCycle(int j, double* out)
{
    return stateDoubleSingle(kFieldPwmDutycycle, j, out);
}
bool DinRailControlBoardNWCRtShm::getDutyCycles(double* outs)
{
    return stateDoubleVector(kFieldPwmDutycycle, outs);
}

// ===========================================================================
// IControlLimits
// ===========================================================================
bool DinRailControlBoardNWCRtShm::setLimits(int axis, double min, double max)
{
    return rpcSetSingleDD(kOpSetLimits, axis, min, max);
}
bool DinRailControlBoardNWCRtShm::getLimits(int axis, double* min, double* max)
{
    return rpcGetSingleDD(kOpGetLimits, axis, min, max);
}
bool DinRailControlBoardNWCRtShm::setVelLimits(int axis, double min, double max)
{
    return rpcSetSingleDD(kOpSetVelLimits, axis, min, max);
}
bool DinRailControlBoardNWCRtShm::getVelLimits(int axis, double* min, double* max)
{
    return rpcGetSingleDD(kOpGetVelLimits, axis, min, max);
}

// ===========================================================================
// IAxisInfo
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getAxisName(int j, std::string& name)
{
    return rpc(
        kOpGetAxisName,
        [&](PayloadWriter& w) { w.putI32(j); },
        [&](PayloadReader& r) { return r.getString(name); });
}
bool DinRailControlBoardNWCRtShm::getJointType(int j, yarp::dev::JointTypeEnum& type)
{
    return rpc(
        kOpGetJointType,
        [&](PayloadWriter& w) { w.putI32(j); },
        [&](PayloadReader& r) {
            std::int32_t v = 0;
            if (!r.getI32(v))
            {
                return false;
            }
            type = static_cast<yarp::dev::JointTypeEnum>(v);
            return true;
        });
}

// ===========================================================================
// IPidControl
// ===========================================================================
bool DinRailControlBoardNWCRtShm::setPid(const PidControlTypeEnum& pidtype, int j, const Pid& pid)
{
    return rpcSimple(kOpSetPid, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putI32(j);
        std::array<double, kPidNumFields> a{};
        pidToArray(pid, a.data());
        for (double d : a)
        {
            w.putF64(d);
        }
    });
}
bool DinRailControlBoardNWCRtShm::setPids(const PidControlTypeEnum& pidtype, const Pid* pids)
{
    return rpcSimple(kOpSetPids, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putU32(m_axes);
        for (std::uint32_t i = 0; i < m_axes; ++i)
        {
            std::array<double, kPidNumFields> a{};
            pidToArray(pids[i], a.data());
            for (double d : a)
            {
                w.putF64(d);
            }
        }
    });
}
bool DinRailControlBoardNWCRtShm::setPidReference(const PidControlTypeEnum& pidtype, int j, double ref)
{
    return rpcSimple(kOpSetPidReference, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putI32(j);
        w.putF64(ref);
    });
}
bool DinRailControlBoardNWCRtShm::setPidReferences(const PidControlTypeEnum& pidtype, const double* refs)
{
    return rpcSimple(kOpSetPidReferences, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putF64Array(refs, m_axes);
    });
}
bool DinRailControlBoardNWCRtShm::setPidErrorLimit(const PidControlTypeEnum& pidtype, int j, double limit)
{
    return rpcSimple(kOpSetPidErrorLimit, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putI32(j);
        w.putF64(limit);
    });
}
bool DinRailControlBoardNWCRtShm::setPidErrorLimits(const PidControlTypeEnum& pidtype, const double* limits)
{
    return rpcSimple(kOpSetPidErrorLimits, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putF64Array(limits, m_axes);
    });
}
bool DinRailControlBoardNWCRtShm::getPidError(const PidControlTypeEnum& pidtype, int j, double* err)
{
    return rpc(
        kOpGetPidError,
        [&](PayloadWriter& w) {
            w.putI32(static_cast<std::int32_t>(pidtype));
            w.putI32(j);
        },
        [&](PayloadReader& r) { return r.getF64(*err); });
}
bool DinRailControlBoardNWCRtShm::getPidErrors(const PidControlTypeEnum& pidtype, double* errs)
{
    return rpc(
        kOpGetPidErrors,
        [&](PayloadWriter& w) { w.putI32(static_cast<std::int32_t>(pidtype)); },
        [&](PayloadReader& r) {
            std::uint32_t n = 0;
            return r.getF64Array(errs, m_axes, n);
        });
}
bool DinRailControlBoardNWCRtShm::getPid(const PidControlTypeEnum& pidtype, int j, Pid* pid)
{
    return rpc(
        kOpGetPid,
        [&](PayloadWriter& w) {
            w.putI32(static_cast<std::int32_t>(pidtype));
            w.putI32(j);
        },
        [&](PayloadReader& r) {
            std::array<double, kPidNumFields> a{};
            for (double& d : a)
            {
                if (!r.getF64(d))
                {
                    return false;
                }
            }
            arrayToPid(a.data(), *pid);
            return true;
        });
}
bool DinRailControlBoardNWCRtShm::getPids(const PidControlTypeEnum& pidtype, Pid* pids)
{
    return rpc(
        kOpGetPids,
        [&](PayloadWriter& w) { w.putI32(static_cast<std::int32_t>(pidtype)); },
        [&](PayloadReader& r) {
            std::uint32_t n = 0;
            if (!r.getU32(n) || n != m_axes)
            {
                return false;
            }
            for (std::uint32_t i = 0; i < n; ++i)
            {
                std::array<double, kPidNumFields> a{};
                for (double& d : a)
                {
                    if (!r.getF64(d))
                    {
                        return false;
                    }
                }
                arrayToPid(a.data(), pids[i]);
            }
            return true;
        });
}
bool DinRailControlBoardNWCRtShm::getPidReference(const PidControlTypeEnum& pidtype, int j, double* ref)
{
    return rpc(
        kOpGetPidReference,
        [&](PayloadWriter& w) {
            w.putI32(static_cast<std::int32_t>(pidtype));
            w.putI32(j);
        },
        [&](PayloadReader& r) { return r.getF64(*ref); });
}
bool DinRailControlBoardNWCRtShm::getPidReferences(const PidControlTypeEnum& pidtype, double* refs)
{
    return rpc(
        kOpGetPidReferences,
        [&](PayloadWriter& w) { w.putI32(static_cast<std::int32_t>(pidtype)); },
        [&](PayloadReader& r) {
            std::uint32_t n = 0;
            return r.getF64Array(refs, m_axes, n);
        });
}
bool DinRailControlBoardNWCRtShm::getPidErrorLimit(const PidControlTypeEnum& pidtype, int j, double* limit)
{
    return rpc(
        kOpGetPidErrorLimit,
        [&](PayloadWriter& w) {
            w.putI32(static_cast<std::int32_t>(pidtype));
            w.putI32(j);
        },
        [&](PayloadReader& r) { return r.getF64(*limit); });
}
bool DinRailControlBoardNWCRtShm::getPidErrorLimits(const PidControlTypeEnum& pidtype, double* limits)
{
    return rpc(
        kOpGetPidErrorLimits,
        [&](PayloadWriter& w) { w.putI32(static_cast<std::int32_t>(pidtype)); },
        [&](PayloadReader& r) {
            std::uint32_t n = 0;
            return r.getF64Array(limits, m_axes, n);
        });
}
bool DinRailControlBoardNWCRtShm::resetPid(const PidControlTypeEnum& pidtype, int j)
{
    return rpcSimple(kOpResetPid, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putI32(j);
    });
}
bool DinRailControlBoardNWCRtShm::disablePid(const PidControlTypeEnum& pidtype, int j)
{
    return rpcSimple(kOpDisablePid, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putI32(j);
    });
}
bool DinRailControlBoardNWCRtShm::enablePid(const PidControlTypeEnum& pidtype, int j)
{
    return rpcSimple(kOpEnablePid, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putI32(j);
    });
}
bool DinRailControlBoardNWCRtShm::isPidEnabled(const PidControlTypeEnum& pidtype, int j, bool* enabled)
{
    return rpc(
        kOpIsPidEnabled,
        [&](PayloadWriter& w) {
            w.putI32(static_cast<std::int32_t>(pidtype));
            w.putI32(j);
        },
        [&](PayloadReader& r) { return r.getBool(*enabled); });
}
bool DinRailControlBoardNWCRtShm::getPidOutput(const PidControlTypeEnum& pidtype, int j, double* out)
{
    return rpc(
        kOpGetPidOutput,
        [&](PayloadWriter& w) {
            w.putI32(static_cast<std::int32_t>(pidtype));
            w.putI32(j);
        },
        [&](PayloadReader& r) { return r.getF64(*out); });
}
bool DinRailControlBoardNWCRtShm::getPidOutputs(const PidControlTypeEnum& pidtype, double* outs)
{
    return rpc(
        kOpGetPidOutputs,
        [&](PayloadWriter& w) { w.putI32(static_cast<std::int32_t>(pidtype)); },
        [&](PayloadReader& r) {
            std::uint32_t n = 0;
            return r.getF64Array(outs, m_axes, n);
        });
}
bool DinRailControlBoardNWCRtShm::setPidOffset(const PidControlTypeEnum& pidtype, int j, double v)
{
    return rpcSimple(kOpSetPidOffset, [&](PayloadWriter& w) {
        w.putI32(static_cast<std::int32_t>(pidtype));
        w.putI32(j);
        w.putF64(v);
    });
}

// ===========================================================================
// IRemoteVariables (not supported over rtshm in v1)
// ===========================================================================
bool DinRailControlBoardNWCRtShm::getRemoteVariable(std::string, yarp::os::Bottle&)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::setRemoteVariable(std::string, const yarp::os::Bottle&)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getRemoteVariablesList(yarp::os::Bottle*)
{
    return false;
}

// ===========================================================================
// IAmplifierControl (not supported over rtshm in v1)
// ===========================================================================
bool DinRailControlBoardNWCRtShm::enableAmp(int)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::disableAmp(int)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getAmpStatus(int*)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getAmpStatus(int, int*)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::setMaxCurrent(int, double)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getMaxCurrent(int, double*)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getNominalCurrent(int, double*)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::setNominalCurrent(int, const double)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getPeakCurrent(int, double*)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::setPeakCurrent(int, const double)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getPWM(int, double*)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getPWMLimit(int, double*)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::setPWMLimit(int, const double)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::getPowerSupplyVoltage(int, double*)
{
    return false;
}

// ===========================================================================
// IControlCalibration / IRemoteCalibrator (not supported over rtshm in v1)
// ===========================================================================
bool DinRailControlBoardNWCRtShm::calibrateRobot()
{
    return false;
}
bool DinRailControlBoardNWCRtShm::abortCalibration()
{
    return false;
}
bool DinRailControlBoardNWCRtShm::abortPark()
{
    return false;
}
bool DinRailControlBoardNWCRtShm::park(bool)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::calibrateAxisWithParams(int, unsigned int, double, double, double)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::setCalibrationParameters(int, const CalibrationParameters&)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::calibrationDone(int)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::isCalibratorDevicePresent(bool* isCalib)
{
    if (isCalib != nullptr)
    {
        *isCalib = false;
    }
    return false;
}
bool DinRailControlBoardNWCRtShm::calibrateSingleJoint(int)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::calibrateWholePart()
{
    return false;
}
bool DinRailControlBoardNWCRtShm::homingSingleJoint(int)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::homingWholePart()
{
    return false;
}
bool DinRailControlBoardNWCRtShm::parkSingleJoint(int, bool)
{
    return false;
}
bool DinRailControlBoardNWCRtShm::parkWholePart()
{
    return false;
}
bool DinRailControlBoardNWCRtShm::quitCalibrate()
{
    return false;
}
bool DinRailControlBoardNWCRtShm::quitPark()
{
    return false;
}
