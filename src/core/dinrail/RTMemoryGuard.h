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
 * @brief Process-wide real-time memory preparation guard.
 *
 * Intended usage:
 *
 * 1. Construct once during application startup.
 * 2. Load plugins.
 * 3. Activate before starting RT cyclic threads.
 * 4. Each RT thread should call:
 *
 *      RTMemoryGuard::prefaultCurrentThreadStack();
 *
 *    before entering the deterministic loop.
 *
 * Linux implementation targets PREEMPT_RT style systems:
 *
 * - Locks process memory using mlockall().
 * - Disables glibc mmap allocations.
 * - Prevents malloc trimming.
 * - Disables Transparent Huge Pages.
 * - Requests minimum CPU latency through cpu_dma_latency.
 * - Warms the heap allocator.
 *
 * This class does NOT make arbitrary future allocations deterministic.
 * RT code should still avoid malloc/new after entering the cyclic loop.
 *
 * Not thread-safe: activate() must complete before any RT thread starts.
 */
class RTMemoryGuard
{
public:
    /**
     * @param stackPrefaultBytes
     *        Stack size to touch for the calling thread during activation.
     *        Must stay below the actual thread stack size, or the prefault
     *        itself will overflow the stack. Default: 0. Later, each RT thread should call prefaultCurrentThreadStack() with a safe value.
     *
     * @param heapReserveBytes
     *        Heap pages to fault in and return to allocator arena.
     *        Default: 128 MiB.
     *
     * @param activateNow
     *        If true, activate immediately.
     */
    explicit RTMemoryGuard(std::size_t stackPrefaultBytes = 0,
                            std::size_t heapReserveBytes = 128 * 1024 * 1024,
                            bool activateNow = false);

    ~RTMemoryGuard();

    RTMemoryGuard(const RTMemoryGuard &) = delete;
    RTMemoryGuard &operator=(const RTMemoryGuard &) = delete;

    RTMemoryGuard(RTMemoryGuard &&other) noexcept;
    RTMemoryGuard &operator=(RTMemoryGuard &&other) noexcept;

    /**
     * @brief Apply the RT-safe memory settings.
     *
     * Calling this method multiple times is safe; after the first successful call it is a no-op.
     *
     * @return true if memory locking succeeded, false otherwise.
     */
    bool activate() noexcept;

    bool isLocked() const noexcept;
    bool isHeapReserved() const noexcept;
    const std::optional<std::string> &
    lastError() const noexcept;

    /**
     * @brief Prefault stack pages of the calling thread.
     *
     * Must be called from every RT thread after pthread_create()
     * and before entering the cyclic control loop.
     *
     * This avoids first-use stack page faults. Caller must ensure
     * bytes stays comfortably under the thread's actual stack size.
     */
    static void prefaultCurrentThreadStack(std::size_t bytes = 512 * 1024) noexcept;

private:
    std::size_t m_stackPrefaultBytes;
    std::size_t m_heapReserveBytes;
    bool m_isActive{false};
    bool m_locked{false};
    bool m_heapReserved{false};
    std::optional<std::string> m_lastError;

#if defined(__linux__)
    int m_dmaLatencyFd{-1};
#endif

private:
    void setError(const char *what, int errorCode) noexcept;
    void setError(const char *what) noexcept;
    bool lockProcessMemory() noexcept;
    void disableMallocPageFaults() noexcept;
    void disableTransparentHugePages() noexcept;
    void requestZeroLatency() noexcept;
    bool reserveHeap(std::size_t bytes) noexcept;

    void unlock() noexcept;

    static std::size_t queryPageSize() noexcept;
};

} // namespace dinrail

#endif // DINRAIL_RTMEMORYGUARD_H