/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Linux transport primitives for the rtshm control-board devices: an anonymous
// shared-memory region backed by memfd_create (transferred over a Unix-domain
// socket via SCM_RIGHTS) and a SOCK_SEQPACKET management connection. libc only.

#ifndef DINRAIL_RTSHM_TRANSPORT_H
#define DINRAIL_RTSHM_TRANSPORT_H

#include "RtShmLayout.h"
#include "RtShmProtocol.h"

#include <cstdint>
#include <ctime>
#include <string>

namespace dinrail::rtshm {

inline std::uint64_t nowNs()
{
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<std::uint64_t>(ts.tv_sec) * 1000000000ull
           + static_cast<std::uint64_t>(ts.tv_nsec);
}

// Derives a deterministic filesystem socket path from a device/port name so the
// NWS and NWC agree without extra configuration.
std::string socketPathForName(const std::string& name);

// ---------------------------------------------------------------------------
// Shared-memory region (memfd backed).
// ---------------------------------------------------------------------------
class SharedRegion
{
public:
    SharedRegion() = default;
    ~SharedRegion();
    SharedRegion(const SharedRegion&) = delete;
    SharedRegion& operator=(const SharedRegion&) = delete;

    // Server side: create, size, map, initialise the header and seal.
    bool create(std::uint32_t axisCount, std::uint64_t generation, bool lockMemory);

    // Client side: adopt a received memfd descriptor and validate the header.
    bool adopt(int fd, std::uint32_t expectedAxisCount, std::uint64_t expectedGeneration);

    void close();

    void* base() const
    {
        return m_base;
    }
    int fd() const
    {
        return m_fd;
    }
    const Layout& layout() const
    {
        return m_layout;
    }
    std::uint32_t axisCount() const
    {
        return m_layout.axisCount;
    }

private:
    int m_fd{-1};
    void* m_base{nullptr};
    std::uint64_t m_bytes{0};
    Layout m_layout{};
    bool m_owner{false};
};

// ---------------------------------------------------------------------------
// A connected management channel (used by both server and client once paired).
// ---------------------------------------------------------------------------
class UnixConnection
{
public:
    UnixConnection() = default;
    explicit UnixConnection(int fd)
        : m_fd(fd)
    {
    }
    ~UnixConnection();
    UnixConnection(const UnixConnection&) = delete;
    UnixConnection& operator=(const UnixConnection&) = delete;
    UnixConnection(UnixConnection&& o) noexcept;
    UnixConnection& operator=(UnixConnection&& o) noexcept;

    bool valid() const
    {
        return m_fd >= 0;
    }
    int fd() const
    {
        return m_fd;
    }
    void close();

    // Send a full RPC packet (header + payload) as a single datagram.
    bool sendPacket(const RpcPacketHeader& hdr, const std::uint8_t* payload);
    // Receive a full RPC packet. payload buffer must be at least kMaxRpcPacket.
    bool recvPacket(RpcPacketHeader& hdr, std::uint8_t* payload, std::uint32_t& payloadBytes);

    // Send a packet together with a file descriptor (SCM_RIGHTS).
    bool sendPacketWithFd(const RpcPacketHeader& hdr, const std::uint8_t* payload, int fdToSend);
    // Receive a packet plus a file descriptor.
    bool recvPacketWithFd(RpcPacketHeader& hdr,
                          std::uint8_t* payload,
                          std::uint32_t& payloadBytes,
                          int& receivedFd);

    // Set a receive timeout in milliseconds (0 disables).
    bool setRecvTimeout(int ms);

    // True if the last recv failed because the peer closed the connection (as
    // opposed to a timeout).
    bool peerClosed() const
    {
        return m_peerClosed;
    }

private:
    int m_fd{-1};
    bool m_peerClosed{false};
};

// ---------------------------------------------------------------------------
// Listening server socket. Accepts a single client.
// ---------------------------------------------------------------------------
class UnixSocketServer
{
public:
    UnixSocketServer() = default;
    ~UnixSocketServer();
    UnixSocketServer(const UnixSocketServer&) = delete;
    UnixSocketServer& operator=(const UnixSocketServer&) = delete;

    bool listen(const std::string& path);
    // Accept a client. Returns an invalid connection on error/timeout.
    UnixConnection accept();
    void close();

    int fd() const
    {
        return m_fd;
    }

private:
    int m_fd{-1};
    std::string m_path;
};

// Connect to a listening server. Returns an invalid connection on failure.
UnixConnection connectUnixSocket(const std::string& path, int timeoutMs);

} // namespace dinrail::rtshm

#endif // DINRAIL_RTSHM_TRANSPORT_H
