/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "RtShmTransport.h"

#include <cctype>
#include <cerrno>
#include <cstring>
#include <functional>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#ifndef MFD_ALLOW_SEALING
#include <linux/memfd.h>
#endif

namespace dinrail::rtshm {

std::string socketPathForName(const std::string& name)
{
    std::string sanitized;
    sanitized.reserve(name.size());
    for (char c : name)
    {
        sanitized.push_back((std::isalnum(static_cast<unsigned char>(c)) != 0) ? c : '_');
    }
    if (sanitized.empty())
    {
        sanitized = "default";
    }

    const char* runtimeDir = ::getenv("XDG_RUNTIME_DIR");
    std::string dir = (runtimeDir != nullptr && runtimeDir[0] != '\0') ? runtimeDir : "/tmp";
    dir += "/dinrail";
    ::mkdir(dir.c_str(), 0700);

    // Keep well within the sockaddr_un path limit (108 bytes).
    std::string path = dir + "/" + sanitized + ".sock";
    if (path.size() >= sizeof(sockaddr_un::sun_path))
    {
        // Fall back to a short hashed name.
        std::size_t h = std::hash<std::string>{}(name);
        path = dir + "/rtshm_" + std::to_string(h) + ".sock";
    }
    return path;
}

// ---------------------------------------------------------------------------
// SharedRegion
// ---------------------------------------------------------------------------
SharedRegion::~SharedRegion()
{
    close();
}

bool SharedRegion::create(std::uint32_t axisCount, std::uint64_t generation, bool lockMemory)
{
    close();
    m_layout = Layout::compute(axisCount);
    m_bytes = m_layout.regionBytes;

    int fd = ::memfd_create("dinrail_rtshm", MFD_ALLOW_SEALING | MFD_CLOEXEC);
    if (fd < 0)
    {
        return false;
    }
    if (::ftruncate(fd, static_cast<off_t>(m_bytes)) != 0)
    {
        ::close(fd);
        return false;
    }
    void* base = ::mmap(nullptr, m_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED)
    {
        ::close(fd);
        return false;
    }

    std::memset(base, 0, m_bytes);

    RegionHeader* h = header(base);
    std::memcpy(h->magic, kRegionMagic, sizeof(kRegionMagic));
    h->abi_major = kAbiMajor;
    h->abi_minor = kAbiMinor;
    h->header_bytes = sizeof(RegionHeader);
    h->region_bytes = m_bytes;
    h->generation = generation;
    h->axis_count = axisCount;
    h->state_slot_count = kStateSlotCount;
    h->cache_line_bytes = kCacheLine;
    h->native_endian_marker = kNativeEndianMarker;
    h->server_heartbeat_offset = m_layout.serverHeartbeatOffset;
    h->client_heartbeat_offset = m_layout.clientHeartbeatOffset;
    h->state_offset = m_layout.stateSlotsOffset;
    h->state_slot_stride = m_layout.stateSlotStride;
    h->mailbox_region_offset = m_layout.mailboxOffset[0];
    h->diagnostics_offset = m_layout.diagnosticsOffset;

    // Prefault every page so no minor faults happen on the hot path.
    volatile std::uint8_t* p = static_cast<volatile std::uint8_t*>(base);
    for (std::uint64_t i = 0; i < m_bytes; i += 4096)
    {
        p[i] = p[i];
    }

    if (lockMemory)
    {
        ::mlock(base, m_bytes);
    }

    // Prevent later resizing. Do not seal writes: both peers update their sections.
    ::fcntl(fd, F_ADD_SEALS, F_SEAL_GROW | F_SEAL_SHRINK);

    m_fd = fd;
    m_base = base;
    m_owner = true;
    return true;
}

bool SharedRegion::adopt(int fd,
                         std::uint32_t expectedAxisCount,
                         std::uint64_t expectedGeneration)
{
    close();
    struct stat st{};
    if (::fstat(fd, &st) != 0)
    {
        ::close(fd);
        return false;
    }
    std::uint64_t bytes = static_cast<std::uint64_t>(st.st_size);
    void* base = ::mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED)
    {
        ::close(fd);
        return false;
    }

    RegionHeader* h = header(base);
    bool ok = std::memcmp(h->magic, kRegionMagic, sizeof(kRegionMagic)) == 0
              && h->abi_major == kAbiMajor && h->region_bytes == bytes
              && h->native_endian_marker == kNativeEndianMarker
              && h->axis_count == expectedAxisCount && h->generation == expectedGeneration;
    if (!ok)
    {
        ::munmap(base, bytes);
        ::close(fd);
        return false;
    }

    m_layout = Layout::compute(h->axis_count);
    if (m_layout.regionBytes != bytes)
    {
        ::munmap(base, bytes);
        ::close(fd);
        return false;
    }

    // Prefault our mapping too.
    volatile std::uint8_t* p = static_cast<volatile std::uint8_t*>(base);
    for (std::uint64_t i = 0; i < bytes; i += 4096)
    {
        p[i] = p[i];
    }

    m_fd = fd;
    m_base = base;
    m_bytes = bytes;
    m_owner = false;
    return true;
}

void SharedRegion::close()
{
    if (m_base != nullptr)
    {
        ::munmap(m_base, m_bytes);
        m_base = nullptr;
    }
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
    m_bytes = 0;
    m_owner = false;
}

// ---------------------------------------------------------------------------
// UnixConnection
// ---------------------------------------------------------------------------
UnixConnection::~UnixConnection()
{
    close();
}

UnixConnection::UnixConnection(UnixConnection&& o) noexcept
    : m_fd(o.m_fd)
{
    o.m_fd = -1;
}

UnixConnection& UnixConnection::operator=(UnixConnection&& o) noexcept
{
    if (this != &o)
    {
        close();
        m_fd = o.m_fd;
        o.m_fd = -1;
    }
    return *this;
}

void UnixConnection::close()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool UnixConnection::setRecvTimeout(int ms)
{
    if (m_fd < 0)
    {
        return false;
    }
    struct timeval tv{};
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return ::setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
}

bool UnixConnection::sendPacket(const RpcPacketHeader& hdr, const std::uint8_t* payload)
{
    if (m_fd < 0 || hdr.payload_bytes > kMaxRpcPacket)
    {
        return false;
    }
    struct iovec iov[2];
    iov[0].iov_base = const_cast<RpcPacketHeader*>(&hdr);
    iov[0].iov_len = sizeof(hdr);
    iov[1].iov_base = const_cast<std::uint8_t*>(payload);
    iov[1].iov_len = hdr.payload_bytes;

    struct msghdr msg{};
    msg.msg_iov = iov;
    msg.msg_iovlen = (hdr.payload_bytes > 0) ? 2 : 1;

    while (true)
    {
        ssize_t n = ::sendmsg(m_fd, &msg, MSG_NOSIGNAL);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return false;
        }
        return true;
    }
}

bool UnixConnection::recvPacket(RpcPacketHeader& hdr,
                                std::uint8_t* payload,
                                std::uint32_t& payloadBytes)
{
    if (m_fd < 0)
    {
        return false;
    }
    struct iovec iov[2];
    iov[0].iov_base = &hdr;
    iov[0].iov_len = sizeof(hdr);
    iov[1].iov_base = payload;
    iov[1].iov_len = kMaxRpcPacket;

    struct msghdr msg{};
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;

    m_peerClosed = false;
    while (true)
    {
        ssize_t n = ::recvmsg(m_fd, &msg, 0);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                m_peerClosed = true;
            }
            return false;
        }
        if (n == 0 || static_cast<std::size_t>(n) < sizeof(hdr))
        {
            m_peerClosed = true;
            return false;
        }
        payloadBytes = static_cast<std::uint32_t>(n) - sizeof(hdr);
        return true;
    }
}

bool UnixConnection::sendPacketWithFd(const RpcPacketHeader& hdr,
                                      const std::uint8_t* payload,
                                      int fdToSend)
{
    if (m_fd < 0)
    {
        return false;
    }
    struct iovec iov[2];
    iov[0].iov_base = const_cast<RpcPacketHeader*>(&hdr);
    iov[0].iov_len = sizeof(hdr);
    iov[1].iov_base = const_cast<std::uint8_t*>(payload);
    iov[1].iov_len = hdr.payload_bytes;

    char control[CMSG_SPACE(sizeof(int))];
    std::memset(control, 0, sizeof(control));

    struct msghdr msg{};
    msg.msg_iov = iov;
    msg.msg_iovlen = (hdr.payload_bytes > 0) ? 2 : 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    std::memcpy(CMSG_DATA(cmsg), &fdToSend, sizeof(int));

    while (true)
    {
        ssize_t n = ::sendmsg(m_fd, &msg, MSG_NOSIGNAL);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return false;
        }
        return true;
    }
}

bool UnixConnection::recvPacketWithFd(RpcPacketHeader& hdr,
                                      std::uint8_t* payload,
                                      std::uint32_t& payloadBytes,
                                      int& receivedFd)
{
    receivedFd = -1;
    if (m_fd < 0)
    {
        return false;
    }
    struct iovec iov[2];
    iov[0].iov_base = &hdr;
    iov[0].iov_len = sizeof(hdr);
    iov[1].iov_base = payload;
    iov[1].iov_len = kMaxRpcPacket;

    char control[CMSG_SPACE(sizeof(int))];
    std::memset(control, 0, sizeof(control));

    struct msghdr msg{};
    msg.msg_iov = iov;
    msg.msg_iovlen = 2;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);

    while (true)
    {
        ssize_t n = ::recvmsg(m_fd, &msg, 0);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return false;
        }
        if (n == 0 || static_cast<std::size_t>(n) < sizeof(hdr))
        {
            return false;
        }
        payloadBytes = static_cast<std::uint32_t>(n) - sizeof(hdr);

        for (struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr;
             cmsg = CMSG_NXTHDR(&msg, cmsg))
        {
            if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS)
            {
                std::memcpy(&receivedFd, CMSG_DATA(cmsg), sizeof(int));
            }
        }
        return true;
    }
}

// ---------------------------------------------------------------------------
// UnixSocketServer
// ---------------------------------------------------------------------------
UnixSocketServer::~UnixSocketServer()
{
    close();
}

bool UnixSocketServer::listen(const std::string& path)
{
    close();
    int fd = ::socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if (fd < 0)
    {
        return false;
    }
    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    if (path.size() >= sizeof(addr.sun_path))
    {
        ::close(fd);
        return false;
    }
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    ::unlink(path.c_str());
    if (::bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        ::close(fd);
        return false;
    }
    ::chmod(path.c_str(), 0600);
    if (::listen(fd, 1) != 0)
    {
        ::close(fd);
        ::unlink(path.c_str());
        return false;
    }
    m_fd = fd;
    m_path = path;
    return true;
}

UnixConnection UnixSocketServer::accept()
{
    if (m_fd < 0)
    {
        return UnixConnection();
    }
    int c = ::accept4(m_fd, nullptr, nullptr, SOCK_CLOEXEC);
    if (c < 0)
    {
        return UnixConnection();
    }
    return UnixConnection(c);
}

void UnixSocketServer::close()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
    if (!m_path.empty())
    {
        ::unlink(m_path.c_str());
        m_path.clear();
    }
}

UnixConnection connectUnixSocket(const std::string& path, int timeoutMs)
{
    int fd = ::socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if (fd < 0)
    {
        return UnixConnection();
    }
    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    if (path.size() >= sizeof(addr.sun_path))
    {
        ::close(fd);
        return UnixConnection();
    }
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    const int stepMs = 5;
    int waited = 0;
    while (true)
    {
        if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0)
        {
            return UnixConnection(fd);
        }
        if (waited >= timeoutMs)
        {
            ::close(fd);
            return UnixConnection();
        }
        struct timespec ts{0, stepMs * 1000000};
        ::nanosleep(&ts, nullptr);
        waited += stepMs;
    }
}

} // namespace dinrail::rtshm
