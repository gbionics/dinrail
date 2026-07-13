/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_RTMEMORYGUARD_H
#define DINRAIL_RTMEMORYGUARD_H

#include <cstddef>
#include <optional>
#include <string>

namespace dinrail
{

/**
 * @brief RAII guard that locks process memory and prefaults stack/heap to
 * eliminate page-fault-induced latency spikes in the RT control loop.
 *
 * Intended for use once at startup of the process, before entering the cyclic
 * real-time tasks, such as the EtherCAT cyclic thread.
 */
class RTMemoryGuard
{
public:
    // Default stack reduced to a safe 512KB to prevent OS hard-limit crashes.
    // Default heap set to 128MB to pre-warm the glibc arena.
    // By default the object is configured but not activated; call activate().
    explicit RTMemoryGuard(std::size_t stackPrefaultBytes = 512 * 1024,
                           std::size_t heapReserveBytes = 128 * 1024 * 1024,
                           bool activateNow = false);

    ~RTMemoryGuard();

    RTMemoryGuard(const RTMemoryGuard&) = delete;
    RTMemoryGuard& operator=(const RTMemoryGuard&) = delete;

    RTMemoryGuard(RTMemoryGuard&& other) noexcept;
    RTMemoryGuard& operator=(RTMemoryGuard&& other) noexcept;

    bool isLocked() const noexcept;
    bool isHeapReserved() const noexcept;
    const std::optional<std::string>& lastError() const noexcept;

    /**
     * @brief Apply the RT-safe memory settings.
     *
     * Calling this method multiple times is safe; after the first successful call it is a no-op.
     *
     * @return true if memory locking succeeded, false otherwise.
     */
    bool activate() noexcept;

private:
    std::size_t m_stackPrefaultBytes{0};
    std::size_t m_heapReserveBytes{0};
    bool m_isActive{false};
    bool m_locked{false};
    bool m_heapReserved{false};
    std::optional<std::string> m_lastError;
#if defined(__linux__)
    int m_dmaLatencyFd{-1};
#endif

    void setError(const char* what, int err) noexcept;
    bool lockProcessMemory() noexcept;
    void disableMallocPageFaults() noexcept;
    void disableTransparentHugePages() noexcept;
    void requestZeroLatency() noexcept;
    void prefaultStack(std::size_t bytes) noexcept;
    static std::size_t queryPageSize() noexcept;
    bool reserveHeap(std::size_t bytes) noexcept;
    void unlock() noexcept;
};

} // namespace dinrail

#endif // DINRAIL_RTMEMORYGUARD_H