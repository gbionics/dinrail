/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Core, RT-safe binary protocol definitions shared by the DinRail rtshm control-board
// devices. This header deliberately does NOT depend on YARP: it only uses the C++
// standard library and fixed-width integer types so that the wire/shared-memory ABI is
// fully explicit and auditable.

#ifndef DINRAIL_RTSHM_PROTOCOL_H
#define DINRAIL_RTSHM_PROTOCOL_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

namespace dinrail::rtshm {

// ---------------------------------------------------------------------------
// Compile-time ABI guarantees (see design doc section 6).
// ---------------------------------------------------------------------------
static_assert(sizeof(double) == 8, "rtshm requires 64-bit doubles");
static_assert(std::numeric_limits<double>::is_iec559, "rtshm requires IEEE-754 doubles");
static_assert(sizeof(std::uint32_t) == 4, "unexpected uint32 size");
static_assert(sizeof(std::uint64_t) == 8, "unexpected uint64 size");
static_assert(sizeof(std::int32_t) == 4, "unexpected int32 size");

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
inline constexpr char kRegionMagic[8] = {'D', 'R', 'R', 'T', 'S', 'H', 'M', '\0'};
inline constexpr std::uint16_t kAbiMajor = 1;
inline constexpr std::uint16_t kAbiMinor = 0;
inline constexpr std::uint32_t kCacheLine = 64;
inline constexpr std::uint32_t kStateSlotCount = 4;
inline constexpr std::uint32_t kRpcMagic = 0x44525243; // "DRRC"
inline constexpr std::uint16_t kRpcMajor = 1;
inline constexpr std::uint16_t kRpcMinor = 0;
inline constexpr std::uint32_t kMaxRpcPacket = 64 * 1024;
inline constexpr std::uint32_t kNativeEndianMarker = 0x01020304;

// ---------------------------------------------------------------------------
// State field indices. Ten double-typed SoA arrays followed by two int32 arrays.
// The order MUST stay stable: it is part of the shared-memory ABI.
// ---------------------------------------------------------------------------
enum StateField : std::uint32_t
{
    kFieldJointPosition = 0,
    kFieldJointVelocity = 1,
    kFieldJointAcceleration = 2,
    kFieldMotorPosition = 3,
    kFieldMotorVelocity = 4,
    kFieldMotorAcceleration = 5,
    kFieldTorque = 6,
    kFieldPwmDutycycle = 7,
    kFieldCurrent = 8,
    kFieldTemperature = 9,
    kNumStateDoubleFields = 10,
    kFieldControlMode = 10,
    kFieldInteractionMode = 11,
    kNumStateFields = 12
};

inline constexpr std::uint64_t stateFieldBit(std::uint32_t field)
{
    return std::uint64_t{1} << field;
}

// ---------------------------------------------------------------------------
// High-rate setpoint mailbox families (see design doc section 8).
// ---------------------------------------------------------------------------
enum MailboxFamily : std::uint32_t
{
    kMailPositionDirect = 0, // 1 array
    kMailVelocity = 1,       // 1 array
    kMailTorque = 2,         // 1 array
    kMailCurrent = 3,        // 1 array
    kMailPwm = 4,            // 1 array
    kMailImpedanceAll = 5,   // 5 arrays: pos, vel, torque, stiffness, damping
    kNumMailboxFamilies = 6
};

inline constexpr std::uint32_t mailboxArrayCount(std::uint32_t family)
{
    return family == kMailImpedanceAll ? 5u : 1u;
}

// ---------------------------------------------------------------------------
// Management-lane binary RPC opcodes. Non real-time; carried over the Unix socket.
// ---------------------------------------------------------------------------
enum RpcOpcode : std::uint32_t
{
    kOpHello = 1, // handshake, server replies with region info and sends memfd

    // IAxisInfo
    kOpGetAxes = 10,
    kOpGetAxisName,
    kOpGetJointType,

    // IEncoders (reset/set only; reads come from the state snapshot)
    kOpResetEncoder = 20,
    kOpResetEncoders,
    kOpSetEncoder,
    kOpSetEncoders,

    // IMotorEncoders
    kOpResetMotorEncoder = 30,
    kOpResetMotorEncoders,
    kOpSetMotorEncoder,
    kOpSetMotorEncoders,
    kOpSetMotorEncoderCountsPerRevolution,
    kOpGetMotorEncoderCountsPerRevolution,
    kOpGetNumberOfMotorEncoders,

    // IMotor
    kOpGetNumberOfMotors = 40,
    kOpGetTemperatureLimit,
    kOpSetTemperatureLimit,
    kOpGetGearboxRatio,
    kOpSetGearboxRatio,

    // IPositionControl
    kOpPositionMoveSingle = 50,
    kOpPositionMoveAll,
    kOpPositionMoveGroup,
    kOpRelativeMoveSingle,
    kOpRelativeMoveAll,
    kOpRelativeMoveGroup,
    kOpCheckMotionDoneSingle,
    kOpCheckMotionDoneAll,
    kOpCheckMotionDoneGroup,
    kOpSetRefSpeedSingle,
    kOpSetRefSpeedsAll,
    kOpSetRefSpeedsGroup,
    kOpSetRefAccelerationSingle,
    kOpSetRefAccelerationsAll,
    kOpSetRefAccelerationsGroup,
    kOpGetRefSpeedSingle,
    kOpGetRefSpeedsAll,
    kOpGetRefSpeedsGroup,
    kOpGetRefAccelerationSingle,
    kOpGetRefAccelerationsAll,
    kOpGetRefAccelerationsGroup,
    kOpGetTargetPositionSingle,
    kOpGetTargetPositionsAll,
    kOpGetTargetPositionsGroup,
    kOpStopSingle,
    kOpStopAll,
    kOpStopGroup,

    // IControlMode
    kOpSetControlModeSingle = 90,
    kOpSetControlModesAll,
    kOpSetControlModesGroup,

    // IInteractionMode
    kOpSetInteractionModeSingle = 100,
    kOpSetInteractionModesAll,
    kOpSetInteractionModesGroup,

    // IVelocityControl
    kOpGetRefVelocitySingle = 110,
    kOpGetRefVelocitiesAll,
    kOpGetRefVelocitiesGroup,

    // ITorqueControl
    kOpGetRefTorqueSingle = 120,
    kOpGetRefTorquesAll,
    kOpGetTorqueRange,
    kOpGetTorqueRanges,
    kOpGetMotorTorqueParams,
    kOpSetMotorTorqueParams,

    // IPWMControl
    kOpGetRefDutyCycleSingle = 130,
    kOpGetRefDutyCyclesAll,

    // ICurrentControl
    kOpGetRefCurrentSingle = 140,
    kOpGetRefCurrentsAll,
    kOpGetCurrentRange,
    kOpGetCurrentRanges,

    // IControlLimits
    kOpSetLimits = 150,
    kOpGetLimits,
    kOpSetVelLimits,
    kOpGetVelLimits,

    // IImpedanceControl
    kOpSetImpedance = 160,
    kOpGetImpedance,
    kOpSetImpedanceOffset,
    kOpGetImpedanceOffset,
    kOpGetCurrentImpedanceLimit,

    // IPidControl
    kOpSetPid = 170,
    kOpSetPids,
    kOpSetPidReference,
    kOpSetPidReferences,
    kOpSetPidErrorLimit,
    kOpSetPidErrorLimits,
    kOpGetPidError,
    kOpGetPidErrors,
    kOpGetPid,
    kOpGetPids,
    kOpGetPidReference,
    kOpGetPidReferences,
    kOpGetPidErrorLimit,
    kOpGetPidErrorLimits,
    kOpResetPid,
    kOpDisablePid,
    kOpEnablePid,
    kOpIsPidEnabled,
    kOpGetPidOutput,
    kOpGetPidOutputs,
    kOpSetPidOffset,

    // IJointFault
    kOpGetLastJointFault = 200,

    // IImpedanceAllSetPointsControl (get; set uses mailbox)
    kOpGetImpedanceAllSetPoints = 210,
};

// ---------------------------------------------------------------------------
// RPC packet header. Sent as the first bytes of every SOCK_SEQPACKET message.
// ---------------------------------------------------------------------------
#pragma pack(push, 1)
struct RpcPacketHeader
{
    std::uint32_t magic;
    std::uint16_t proto_major;
    std::uint16_t proto_minor;
    std::uint32_t opcode;
    std::uint32_t flags;
    std::uint64_t request_id;
    std::uint32_t payload_bytes;
    std::uint32_t reserved;
};
#pragma pack(pop)
static_assert(sizeof(RpcPacketHeader) == 32, "unexpected RpcPacketHeader size");

inline constexpr std::uint32_t kRpcFlagOk = 0x1; // response: call returned true

// ---------------------------------------------------------------------------
// Atomic helpers (see design doc section 6). We operate on plain uint64 fields
// living in shared memory using the compiler atomic builtins.
// ---------------------------------------------------------------------------
inline void storeRelease(std::uint64_t& v, std::uint64_t x)
{
    __atomic_store_n(&v, x, __ATOMIC_RELEASE);
}
inline std::uint64_t loadAcquire(const std::uint64_t& v)
{
    return __atomic_load_n(const_cast<std::uint64_t*>(&v), __ATOMIC_ACQUIRE);
}
inline std::uint64_t loadRelaxed(const std::uint64_t& v)
{
    return __atomic_load_n(const_cast<std::uint64_t*>(&v), __ATOMIC_RELAXED);
}
inline std::uint64_t fetchAdd(std::uint64_t& v, std::uint64_t x)
{
    return __atomic_fetch_add(&v, x, __ATOMIC_ACQ_REL);
}

// ---------------------------------------------------------------------------
// Fixed-capacity binary payload writer/reader used for the management RPC lane.
// No dynamic allocation on the hot path; bounds are always checked.
// ---------------------------------------------------------------------------
class PayloadWriter
{
public:
    PayloadWriter(std::uint8_t* buffer, std::size_t capacity)
        : m_buf(buffer)
        , m_cap(capacity)
    {
    }

    bool putU32(std::uint32_t v)
    {
        return raw(&v, sizeof(v));
    }
    bool putI32(std::int32_t v)
    {
        return raw(&v, sizeof(v));
    }
    bool putU64(std::uint64_t v)
    {
        return raw(&v, sizeof(v));
    }
    bool putF64(double v)
    {
        return raw(&v, sizeof(v));
    }
    bool putBool(bool v)
    {
        std::uint8_t b = v ? 1 : 0;
        return raw(&b, sizeof(b));
    }
    bool putI32Array(const std::int32_t* data, std::uint32_t n)
    {
        if (!putU32(n))
        {
            return false;
        }
        return raw(data, static_cast<std::size_t>(n) * sizeof(std::int32_t));
    }
    bool putF64Array(const double* data, std::uint32_t n)
    {
        if (!putU32(n))
        {
            return false;
        }
        return raw(data, static_cast<std::size_t>(n) * sizeof(double));
    }
    bool putString(const std::string& s)
    {
        if (!putU32(static_cast<std::uint32_t>(s.size())))
        {
            return false;
        }
        return raw(s.data(), s.size());
    }

    std::size_t size() const
    {
        return m_pos;
    }

private:
    bool raw(const void* src, std::size_t len)
    {
        if (m_pos + len > m_cap)
        {
            m_ok = false;
            return false;
        }
        std::memcpy(m_buf + m_pos, src, len);
        m_pos += len;
        return true;
    }

    std::uint8_t* m_buf;
    std::size_t m_cap;
    std::size_t m_pos{0};
    bool m_ok{true};
};

class PayloadReader
{
public:
    PayloadReader(const std::uint8_t* buffer, std::size_t size)
        : m_buf(buffer)
        , m_size(size)
    {
    }

    bool getU32(std::uint32_t& v)
    {
        return raw(&v, sizeof(v));
    }
    bool getI32(std::int32_t& v)
    {
        return raw(&v, sizeof(v));
    }
    bool getU64(std::uint64_t& v)
    {
        return raw(&v, sizeof(v));
    }
    bool getF64(double& v)
    {
        return raw(&v, sizeof(v));
    }
    bool getBool(bool& v)
    {
        std::uint8_t b = 0;
        if (!raw(&b, sizeof(b)))
        {
            return false;
        }
        v = (b != 0);
        return true;
    }
    // Reads a length-prefixed int32 array into caller storage (max maxN elements).
    bool getI32Array(std::int32_t* out, std::uint32_t maxN, std::uint32_t& n)
    {
        if (!getU32(n) || n > maxN)
        {
            return false;
        }
        return raw(out, static_cast<std::size_t>(n) * sizeof(std::int32_t));
    }
    bool getF64Array(double* out, std::uint32_t maxN, std::uint32_t& n)
    {
        if (!getU32(n) || n > maxN)
        {
            return false;
        }
        return raw(out, static_cast<std::size_t>(n) * sizeof(double));
    }
    bool getString(std::string& out)
    {
        std::uint32_t n = 0;
        if (!getU32(n) || m_pos + n > m_size)
        {
            return false;
        }
        out.assign(reinterpret_cast<const char*>(m_buf + m_pos), n);
        m_pos += n;
        return true;
    }

    std::size_t remaining() const
    {
        return m_size - m_pos;
    }

private:
    bool raw(void* dst, std::size_t len)
    {
        if (m_pos + len > m_size)
        {
            return false;
        }
        std::memcpy(dst, m_buf + m_pos, len);
        m_pos += len;
        return true;
    }

    const std::uint8_t* m_buf;
    std::size_t m_size;
    std::size_t m_pos{0};
};

} // namespace dinrail::rtshm

#endif // DINRAIL_RTSHM_PROTOCOL_H
