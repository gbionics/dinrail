/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Shared-memory region layout and the lock-free state-snapshot / setpoint-mailbox
// access primitives (design doc sections 6, 7 and 8). No YARP dependency.

#ifndef DINRAIL_RTSHM_LAYOUT_H
#define DINRAIL_RTSHM_LAYOUT_H

#include "RtShmProtocol.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace dinrail::rtshm {

inline constexpr std::uint64_t alignUp(std::uint64_t v, std::uint64_t a)
{
    return (v + a - 1) / a * a;
}

// ---------------------------------------------------------------------------
// Fixed-layout structures placed directly in shared memory.
// ---------------------------------------------------------------------------
#pragma pack(push, 8)
struct RegionHeader
{
    char magic[8];
    std::uint16_t abi_major;
    std::uint16_t abi_minor;
    std::uint32_t header_bytes;

    std::uint64_t region_bytes;
    std::uint64_t generation;

    std::uint32_t axis_count;
    std::uint32_t state_slot_count;
    std::uint32_t cache_line_bytes;
    std::uint32_t native_endian_marker;

    std::uint64_t server_heartbeat_offset;
    std::uint64_t client_heartbeat_offset;
    std::uint64_t state_offset;
    std::uint64_t state_slot_stride;
    std::uint64_t mailbox_region_offset;
    std::uint64_t diagnostics_offset;
};

struct Heartbeat
{
    std::uint64_t seq;
    std::uint64_t time_ns;
    std::uint64_t reserved[6]; // pad to a full cache line
};

struct StateSlotPrefix
{
    std::uint64_t guard; // even == stable, odd == being written
    std::uint64_t sequence;
    std::uint64_t sample_time_ns;
    std::uint64_t publish_time_ns;
    std::uint64_t valid_fields;
    std::uint32_t axis_count;
    std::uint32_t status;
};

struct StatePublished
{
    std::uint64_t sequence; // last fully-published sequence (release-stored by writer)
    std::uint64_t reserved[7];
};

struct MailboxPrefix
{
    std::uint64_t guard;
    std::uint64_t sequence;
    std::uint64_t publish_time_ns;
    std::uint32_t axis_count;
    std::uint32_t array_count;
};

struct Diagnostics
{
    std::uint64_t state_publications;
    std::uint64_t state_reader_retries;
    std::uint64_t state_reader_failures;
    std::uint64_t setpoints_applied;
    std::uint64_t cycles;
    std::uint64_t cycle_overruns;
    std::uint64_t reserved[10];
};
#pragma pack(pop)

// ---------------------------------------------------------------------------
// Layout: deterministically computed from the axis count on both processes.
// ---------------------------------------------------------------------------
struct Layout
{
    std::uint32_t axisCount{0};
    std::uint64_t headerOffset{0};
    std::uint64_t serverHeartbeatOffset{0};
    std::uint64_t clientHeartbeatOffset{0};
    std::uint64_t statePublishedOffset{0};
    std::uint64_t stateSlotsOffset{0};
    std::uint64_t stateSlotStride{0};
    std::uint64_t mailboxOffset[kNumMailboxFamilies]{};
    std::uint64_t mailboxBytes[kNumMailboxFamilies]{};
    std::uint64_t diagnosticsOffset{0};
    std::uint64_t regionBytes{0};

    static std::uint64_t stateSlotStrideFor(std::uint32_t axis)
    {
        std::uint64_t prefix = alignUp(sizeof(StateSlotPrefix), kCacheLine);
        std::uint64_t doubles = std::uint64_t{kNumStateDoubleFields} * axis * sizeof(double);
        std::uint64_t ints = std::uint64_t{2} * axis * sizeof(std::int32_t);
        return alignUp(prefix + doubles + ints, kCacheLine);
    }

    static std::uint64_t mailboxBytesFor(std::uint32_t family, std::uint32_t axis)
    {
        std::uint64_t prefix = alignUp(sizeof(MailboxPrefix), kCacheLine);
        std::uint64_t arrays
            = std::uint64_t{mailboxArrayCount(family)} * axis * sizeof(double);
        return alignUp(prefix + arrays, kCacheLine);
    }

    static Layout compute(std::uint32_t axis)
    {
        Layout l;
        l.axisCount = axis;
        std::uint64_t off = 0;
        l.headerOffset = off;
        off = alignUp(off + sizeof(RegionHeader), kCacheLine);
        l.serverHeartbeatOffset = off;
        off += sizeof(Heartbeat);
        l.clientHeartbeatOffset = off;
        off += sizeof(Heartbeat);
        l.statePublishedOffset = off;
        off += sizeof(StatePublished);
        off = alignUp(off, kCacheLine);
        l.stateSlotsOffset = off;
        l.stateSlotStride = stateSlotStrideFor(axis);
        off += l.stateSlotStride * kStateSlotCount;
        for (std::uint32_t f = 0; f < kNumMailboxFamilies; ++f)
        {
            off = alignUp(off, kCacheLine);
            l.mailboxOffset[f] = off;
            l.mailboxBytes[f] = mailboxBytesFor(f, axis);
            off += l.mailboxBytes[f];
        }
        off = alignUp(off, kCacheLine);
        l.diagnosticsOffset = off;
        off += sizeof(Diagnostics);
        l.regionBytes = alignUp(off, 4096);
        return l;
    }
};

// ---------------------------------------------------------------------------
// Accessors on top of a mapped base pointer.
// ---------------------------------------------------------------------------
inline RegionHeader* header(void* base)
{
    return reinterpret_cast<RegionHeader*>(base);
}
inline Heartbeat* serverHeartbeat(void* base, const Layout& l)
{
    return reinterpret_cast<Heartbeat*>(static_cast<std::uint8_t*>(base) + l.serverHeartbeatOffset);
}
inline Heartbeat* clientHeartbeat(void* base, const Layout& l)
{
    return reinterpret_cast<Heartbeat*>(static_cast<std::uint8_t*>(base) + l.clientHeartbeatOffset);
}
inline StatePublished* statePublished(void* base, const Layout& l)
{
    return reinterpret_cast<StatePublished*>(static_cast<std::uint8_t*>(base)
                                             + l.statePublishedOffset);
}
inline Diagnostics* diagnostics(void* base, const Layout& l)
{
    return reinterpret_cast<Diagnostics*>(static_cast<std::uint8_t*>(base) + l.diagnosticsOffset);
}
inline StateSlotPrefix* stateSlot(void* base, const Layout& l, std::uint32_t slot)
{
    return reinterpret_cast<StateSlotPrefix*>(static_cast<std::uint8_t*>(base) + l.stateSlotsOffset
                                              + std::uint64_t{slot} * l.stateSlotStride);
}
// Returns a pointer to the k-th double array inside a state slot (k in [0,10)).
inline double* stateSlotDoubleArray(StateSlotPrefix* slot, std::uint32_t axis, std::uint32_t k)
{
    auto* bytes = reinterpret_cast<std::uint8_t*>(slot) + alignUp(sizeof(StateSlotPrefix), kCacheLine);
    return reinterpret_cast<double*>(bytes + std::uint64_t{k} * axis * sizeof(double));
}
// Returns a pointer to the k-th int32 array inside a state slot (k in [0,2)).
inline std::int32_t* stateSlotIntArray(StateSlotPrefix* slot, std::uint32_t axis, std::uint32_t k)
{
    auto* bytes = reinterpret_cast<std::uint8_t*>(slot) + alignUp(sizeof(StateSlotPrefix), kCacheLine);
    bytes += std::uint64_t{kNumStateDoubleFields} * axis * sizeof(double);
    return reinterpret_cast<std::int32_t*>(bytes + std::uint64_t{k} * axis * sizeof(std::int32_t));
}
inline MailboxPrefix* mailbox(void* base, const Layout& l, std::uint32_t family)
{
    return reinterpret_cast<MailboxPrefix*>(static_cast<std::uint8_t*>(base)
                                            + l.mailboxOffset[family]);
}
inline double* mailboxArray(MailboxPrefix* mb, std::uint32_t axis, std::uint32_t arrayIdx)
{
    auto* bytes = reinterpret_cast<std::uint8_t*>(mb) + alignUp(sizeof(MailboxPrefix), kCacheLine);
    return reinterpret_cast<double*>(bytes + std::uint64_t{arrayIdx} * axis * sizeof(double));
}

// ---------------------------------------------------------------------------
// Host-side snapshot mirror used by both producer and consumer for staging.
// ---------------------------------------------------------------------------
struct StateSnapshot
{
    std::uint64_t sequence{0};
    std::uint64_t sampleTimeNs{0};
    std::uint64_t publishTimeNs{0};
    std::uint64_t validFields{0};
    std::uint32_t axisCount{0};
    // 10 double arrays + 2 int32 arrays, indexed by StateField.
    std::vector<double> doubles[kNumStateDoubleFields];
    std::vector<std::int32_t> ints[2];

    void resize(std::uint32_t axis)
    {
        axisCount = axis;
        for (auto& d : doubles)
        {
            d.assign(axis, 0.0);
        }
        for (auto& i : ints)
        {
            i.assign(axis, 0);
        }
    }
};

// Publisher: copy a staged snapshot into the ring using odd/even guards.
inline void publishState(void* base, const Layout& l, const StateSnapshot& snap)
{
    StatePublished* pub = statePublished(base, l);
    std::uint64_t nextSeq = loadRelaxed(pub->sequence) + 1;
    std::uint32_t slotIdx = static_cast<std::uint32_t>(nextSeq % kStateSlotCount);
    StateSlotPrefix* slot = stateSlot(base, l, slotIdx);

    storeRelease(slot->guard, (nextSeq << 1) | 1u); // mark writing

    slot->sequence = nextSeq;
    slot->sample_time_ns = snap.sampleTimeNs;
    slot->publish_time_ns = snap.publishTimeNs;
    slot->valid_fields = snap.validFields;
    slot->axis_count = snap.axisCount;
    slot->status = 0;
    for (std::uint32_t k = 0; k < kNumStateDoubleFields; ++k)
    {
        std::memcpy(stateSlotDoubleArray(slot, snap.axisCount, k),
                    snap.doubles[k].data(),
                    std::size_t{snap.axisCount} * sizeof(double));
    }
    for (std::uint32_t k = 0; k < 2; ++k)
    {
        std::memcpy(stateSlotIntArray(slot, snap.axisCount, k),
                    snap.ints[k].data(),
                    std::size_t{snap.axisCount} * sizeof(std::int32_t));
    }

    storeRelease(slot->guard, nextSeq << 1); // stable
    storeRelease(pub->sequence, nextSeq);
}

// Consumer: read newest snapshot into local mirror. Returns true on success.
// Retries a small bounded number of times if it races the writer.
inline bool readState(void* base, const Layout& l, StateSnapshot& out, int maxRetries = 2)
{
    StatePublished* pub = statePublished(base, l);
    for (int attempt = 0; attempt <= maxRetries; ++attempt)
    {
        std::uint64_t seq = loadAcquire(pub->sequence);
        if (seq == 0)
        {
            return false; // nothing published yet
        }
        std::uint32_t slotIdx = static_cast<std::uint32_t>(seq % kStateSlotCount);
        StateSlotPrefix* slot = stateSlot(base, l, slotIdx);

        std::uint64_t guardBefore = loadAcquire(slot->guard);
        if ((guardBefore & 1u) != 0 || (guardBefore >> 1) != seq)
        {
            continue; // being written or stale slot
        }

        out.sequence = slot->sequence;
        out.sampleTimeNs = slot->sample_time_ns;
        out.publishTimeNs = slot->publish_time_ns;
        out.validFields = slot->valid_fields;
        std::uint32_t axis = slot->axis_count;
        out.axisCount = axis;
        for (std::uint32_t k = 0; k < kNumStateDoubleFields; ++k)
        {
            out.doubles[k].resize(axis);
            std::memcpy(out.doubles[k].data(),
                        stateSlotDoubleArray(slot, axis, k),
                        std::size_t{axis} * sizeof(double));
        }
        for (std::uint32_t k = 0; k < 2; ++k)
        {
            out.ints[k].resize(axis);
            std::memcpy(out.ints[k].data(),
                        stateSlotIntArray(slot, axis, k),
                        std::size_t{axis} * sizeof(std::int32_t));
        }

        std::uint64_t guardAfter = loadAcquire(slot->guard);
        if (guardAfter == guardBefore)
        {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Setpoint mailbox publish/read (design doc section 8).
// ---------------------------------------------------------------------------
inline void publishMailbox(void* base,
                           const Layout& l,
                           std::uint32_t family,
                           std::uint32_t axis,
                           const double* const* arrays,
                           std::uint32_t arrayCount,
                           std::uint64_t timeNs)
{
    MailboxPrefix* mb = mailbox(base, l, family);
    std::uint64_t nextSeq = loadRelaxed(mb->sequence) + 1;
    storeRelease(mb->guard, (nextSeq << 1) | 1u);
    mb->publish_time_ns = timeNs;
    mb->axis_count = axis;
    mb->array_count = arrayCount;
    for (std::uint32_t a = 0; a < arrayCount; ++a)
    {
        std::memcpy(mailboxArray(mb, axis, a),
                    arrays[a],
                    std::size_t{axis} * sizeof(double));
    }
    mb->sequence = nextSeq;
    storeRelease(mb->guard, nextSeq << 1);
}

// Reads the mailbox if its sequence advanced past lastSeq. Returns true if new data
// was consumed. arrays must point to arrayCount buffers of at least axis doubles.
inline bool readMailbox(void* base,
                        const Layout& l,
                        std::uint32_t family,
                        std::uint32_t axis,
                        std::uint64_t& lastSeq,
                        double* const* arrays,
                        std::uint32_t arrayCount)
{
    MailboxPrefix* mb = mailbox(base, l, family);
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        std::uint64_t guardBefore = loadAcquire(mb->guard);
        if ((guardBefore & 1u) != 0)
        {
            continue;
        }
        std::uint64_t seq = mb->sequence;
        if (seq == 0 || seq == lastSeq)
        {
            return false;
        }
        for (std::uint32_t a = 0; a < arrayCount; ++a)
        {
            std::memcpy(arrays[a],
                        mailboxArray(mb, axis, a),
                        std::size_t{axis} * sizeof(double));
        }
        std::uint64_t guardAfter = loadAcquire(mb->guard);
        if (guardAfter == guardBefore)
        {
            lastSeq = seq;
            return true;
        }
    }
    return false;
}

} // namespace dinrail::rtshm

#endif // DINRAIL_RTSHM_LAYOUT_H
