/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <dinrail/RTMemoryGuard.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

#if defined(__linux__)
#    include <errno.h>
#    include <fcntl.h>
#    include <malloc.h>
#    include <sys/mman.h>
#    include <sys/prctl.h>
#    include <unistd.h>
#elif defined(__APPLE__)
#    include <errno.h>
#    include <sys/mman.h>
#    include <unistd.h>
#elif defined(_WIN32)
#    include <malloc.h> // for _alloca
#    include <windows.h>
#endif

namespace dinrail
{

namespace {

#if !defined(_WIN32)
// Touch stack memory by recursing through real stack frames instead of growing a single
// frame with alloca(). A single large alloca() is undone as soon as the function returns
// (so it does not actually keep any pages resident for later use) and, for larger sizes or
// threads with a smaller stack, it can blow straight through the stack guard page with no
// safety margin. Recursing through small, fixed-size frames touches the same number of pages
// while never growing any single frame by more than a page.
constexpr std::size_t kStackTouchFrameSize = 4096;

void touchStackFrames(std::size_t remainingBytes) noexcept
{
    volatile std::uint8_t frame[kStackTouchFrameSize];
    std::memset(const_cast<std::uint8_t*>(frame), 0, sizeof(frame));
    if (remainingBytes > kStackTouchFrameSize) {
        touchStackFrames(remainingBytes - kStackTouchFrameSize);
    }
}
#endif

} // namespace

RTMemoryGuard::RTMemoryGuard(const std::size_t stackPrefaultBytes,
                             const std::size_t heapReserveBytes,
                             const bool activateNow)
        : m_stackPrefaultBytes(stackPrefaultBytes),
          m_heapReserveBytes(heapReserveBytes)
{
    if (activateNow) {
        activate();
    }
}

RTMemoryGuard::~RTMemoryGuard()
{
    unlock();
}

RTMemoryGuard::RTMemoryGuard(RTMemoryGuard&& other) noexcept
        : m_stackPrefaultBytes(other.m_stackPrefaultBytes),
          m_heapReserveBytes(other.m_heapReserveBytes),
          m_isActive(other.m_isActive),
          m_locked(other.m_locked),
          m_heapReserved(other.m_heapReserved),
          m_lastError(std::move(other.m_lastError))
{
#if defined(__linux__)
    m_dmaLatencyFd = other.m_dmaLatencyFd;
    other.m_dmaLatencyFd = -1;
#endif
    other.m_stackPrefaultBytes = 0;
    other.m_heapReserveBytes = 0;
    other.m_isActive = false;
    other.m_locked = false;
    other.m_heapReserved = false;
}

RTMemoryGuard& RTMemoryGuard::operator=(RTMemoryGuard&& other) noexcept
{
    if (this != &other) {
        unlock();
        m_stackPrefaultBytes = other.m_stackPrefaultBytes;
        m_heapReserveBytes = other.m_heapReserveBytes;
        m_isActive = other.m_isActive;
        m_locked = other.m_locked;
        m_heapReserved = other.m_heapReserved;
        m_lastError = std::move(other.m_lastError);
#if defined(__linux__)
        m_dmaLatencyFd = other.m_dmaLatencyFd;
        other.m_dmaLatencyFd = -1;
#endif
        other.m_stackPrefaultBytes = 0;
        other.m_heapReserveBytes = 0;
        other.m_isActive = false;
        other.m_locked = false;
        other.m_heapReserved = false;
    }
    return *this;
}

bool RTMemoryGuard::isLocked() const noexcept
{
    return m_locked;
}

bool RTMemoryGuard::isHeapReserved() const noexcept
{
    return m_heapReserved;
}

const std::optional<std::string>& RTMemoryGuard::lastError() const noexcept
{
    return m_lastError;
}

bool RTMemoryGuard::activate() noexcept
{
    if (m_isActive) {
        return m_locked;
    }

    disableMallocPageFaults();
    disableTransparentHugePages();
    requestZeroLatency();

    if (m_stackPrefaultBytes > 0) {
        prefaultStack(m_stackPrefaultBytes);
    }

    if (m_heapReserveBytes > 0) {
        m_heapReserved = reserveHeap(m_heapReserveBytes);
    }

    m_locked = lockProcessMemory();
    m_isActive = true;
    return m_locked;
}

void RTMemoryGuard::setError(const char* what, int err) noexcept
{
    m_lastError = std::string(what) + ": " + std::strerror(err);
}

bool RTMemoryGuard::lockProcessMemory() noexcept
{
#if defined(__linux__)
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        setError("mlockall(MCL_CURRENT | MCL_FUTURE)", errno);
        std::fprintf(stderr,
                     "RTMemoryGuard: Failed to lock process memory: %s\n",
                     m_lastError->c_str());
        return false;
    }
    return true;

#elif defined(__APPLE__) || defined(_WIN32)
    return true;
#else
    return true;
#endif
}

void RTMemoryGuard::disableMallocPageFaults() noexcept
{
#if defined(__linux__)
    if (mallopt(M_MMAP_MAX, 0) != 1) {
        setError("mallopt(M_MMAP_MAX)", errno);
        std::fprintf(stderr,
                     "RTMemoryGuard: Failed to set M_MMAP_MAX: %s\n",
                     m_lastError->c_str());
    }
    if (mallopt(M_TRIM_THRESHOLD, -1) != 1) {
        setError("mallopt(M_TRIM_THRESHOLD)", errno);
        std::fprintf(stderr,
                     "RTMemoryGuard: Failed to set M_TRIM_THRESHOLD: %s\n",
                     m_lastError->c_str());
    }
#endif
}

void RTMemoryGuard::disableTransparentHugePages() noexcept
{
#if defined(__linux__) && defined(PR_SET_THP_DISABLE)
    if (prctl(PR_SET_THP_DISABLE, 1, 0, 0, 0) != 0) {
        setError("prctl(PR_SET_THP_DISABLE)", errno);
        std::fprintf(stderr,
                     "RTMemoryGuard: Failed to disable transparent huge pages: %s\n",
                     m_lastError->c_str());
    }
#endif
}

void RTMemoryGuard::requestZeroLatency() noexcept
{
#if defined(__linux__)
    m_dmaLatencyFd = open("/dev/cpu_dma_latency", O_RDWR);
    if (m_dmaLatencyFd >= 0) {
        const int32_t latency_target = 0;
        if (write(m_dmaLatencyFd, &latency_target, sizeof(latency_target)) < 0) {
            setError("write(/dev/cpu_dma_latency)", errno);
            std::fprintf(stderr,
                         "RTMemoryGuard: Failed to write to /dev/cpu_dma_latency: %s\n",
                         m_lastError->c_str());
        }
    } else {
        setError("open(/dev/cpu_dma_latency)", errno);
        std::fprintf(stderr,
                     "RTMemoryGuard: Failed to open /dev/cpu_dma_latency: %s\n",
                     m_lastError->c_str());
    }
#endif
}

void RTMemoryGuard::prefaultStack(std::size_t bytes) noexcept
{
#if defined(_WIN32)
    volatile std::uint8_t* dummy = static_cast<volatile std::uint8_t*>(_alloca(bytes));
    const std::size_t pageSize = queryPageSize();
    for (std::size_t i = 0; i < bytes; i += pageSize) {
        dummy[i] = 0;
    }
#else
    // Touch pages by recursing through real stack frames (see touchStackFrames() above)
    // instead of a single alloca(bytes) call, which is reclaimed the instant this function
    // returns and offers no protection against blowing through the stack guard page.
    touchStackFrames(bytes);
#endif
}

std::size_t RTMemoryGuard::queryPageSize() noexcept
{
#if defined(__linux__) || defined(__APPLE__)
    const long p = sysconf(_SC_PAGESIZE);
    return p > 0 ? static_cast<std::size_t>(p) : 4096;
#elif defined(_WIN32)
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
#else
    return 4096;
#endif
}

bool RTMemoryGuard::reserveHeap(std::size_t bytes) noexcept
{
    // IMPROVEMENT 3: Do not keep the pointer. Allocate, touch, and free immediately.
    const std::size_t pageSize = queryPageSize();
    std::size_t heapSize = ((bytes + pageSize - 1) / pageSize) * pageSize;
    void* ptr = nullptr;

#if defined(_WIN32)
    ptr = VirtualAlloc(nullptr, heapSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!ptr) {
        setError("VirtualAlloc", static_cast<int>(GetLastError()));
        return false;
    }
#else
    if (posix_memalign(&ptr, pageSize, heapSize) != 0) {
        setError("posix_memalign", errno);
        std::fprintf(stderr,
                     "RTMemoryGuard: Failed to allocate heap memory: %s\n",
                     m_lastError->c_str());
        return false;
    }
#endif

    volatile std::uint8_t* p = static_cast<volatile std::uint8_t*>(ptr);
    for (std::size_t i = 0; i < heapSize; i += pageSize) {
        p[i] = 0;
    }

#if defined(_WIN32)
    if (!VirtualLock(ptr, heapSize)) {
        setError("VirtualLock", static_cast<int>(GetLastError()));
    }
    VirtualUnlock(ptr, heapSize);
    VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(__APPLE__)
    if (mlock(ptr, heapSize) != 0) {
        setError("mlock", errno);
    }
    munlock(ptr, heapSize);
    std::free(ptr);
#else
    // Linux returns the pre-faulted RAM to glibc's arena because of mallopt settings.
    std::free(ptr);
#endif

    return true;
}

void RTMemoryGuard::unlock() noexcept
{
    if (!m_isActive) {
        return;
    }

#if defined(__linux__) || defined(__APPLE__)
    if (m_locked) {
        munlockall();
    }
#endif

#if defined(__linux__)
    if (m_dmaLatencyFd >= 0) {
        close(m_dmaLatencyFd);
        m_dmaLatencyFd = -1;
    }
#endif

    m_isActive = false;
    m_locked = false;
    m_heapReserved = false;
}

} // namespace dinrail
