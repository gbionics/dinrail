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
#include <alloca.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <alloca.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <malloc.h> // for _alloca
#include <windows.h>
#endif

namespace dinrail
{

namespace
{

    constexpr std::size_t kDefaultPageSize = 4096;

#if defined(__linux__)

    // RLIMIT_MEMLOCK only bounds the heap reserve, not the full mlockall()
    // footprint (stack, code, all future mappings). It is a cheap early-out,
    // not a guarantee that mlockall() will succeed.
    bool checkMemoryLimit(std::size_t bytes)
    {
        struct rlimit limit{};

        if (getrlimit(RLIMIT_MEMLOCK, &limit) != 0) {
            return true;
        }

        if (limit.rlim_cur == RLIM_INFINITY){
            return true;
        }

        return bytes <= limit.rlim_cur;
    }

#endif

#if defined(__linux__) || defined(__APPLE__)

    // Hard ceiling on a single prefault request. Guards against a bad
    // caller-supplied byte count silently overflowing the thread stack.
    constexpr std::size_t kMaxStackPrefaultBytes = 8 * 1024 * 1024;

    // alloca() allocations inside a loop persist until the enclosing
    // function returns, so a loop grows the same stack region a
    // recursive version would, without recursion depth or call overhead.
    void touchStackIterative(std::size_t bytes, std::size_t pageSize)
    {
        bytes = bytes < kMaxStackPrefaultBytes ? bytes : kMaxStackPrefaultBytes;

        for (std::size_t touched = 0; touched < bytes; touched += pageSize)
        {
            volatile std::uint8_t *page =
                static_cast<volatile std::uint8_t *>(alloca(pageSize));
            page[0] = 0;
        }
    }

#endif

}

RTMemoryGuard::RTMemoryGuard(std::size_t stackPrefaultBytes,
                             std::size_t heapReserveBytes, bool activateNow)
                            : m_stackPrefaultBytes(stackPrefaultBytes),
                              m_heapReserveBytes(heapReserveBytes)
{
    if (activateNow)
        activate();
}

RTMemoryGuard::~RTMemoryGuard()
{
    unlock();
}

RTMemoryGuard::RTMemoryGuard(RTMemoryGuard &&other) noexcept
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
    other.m_isActive = false;
    other.m_locked = false;
    other.m_heapReserved = false;
}

RTMemoryGuard &RTMemoryGuard::operator=(RTMemoryGuard &&other) noexcept
{
    if (this == &other)
        return *this;

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
    other.m_isActive = false;
    other.m_locked = false;
    other.m_heapReserved = false;

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

const std::optional<std::string> &RTMemoryGuard::lastError() const noexcept
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

    if (!lockProcessMemory()) {
        // Release anything already acquired above (e.g. the latency fd)
        // so a failed activation does not leak resources or pin the
        // zero-latency constraint for the process lifetime.
        unlock();
        return false;
    }

    if (m_heapReserveBytes > 0) {
        m_heapReserved = reserveHeap(m_heapReserveBytes);
    }
    if (m_stackPrefaultBytes > 0) {
        prefaultCurrentThreadStack(m_stackPrefaultBytes);
    }

    m_locked = true;
    m_isActive = true;

    return true;
}

void RTMemoryGuard::setError(const char *what, int errorCode) noexcept
{
    m_lastError = std::string(what) + ": " + std::strerror(errorCode);
}

void RTMemoryGuard::setError(const char *what) noexcept
{
    m_lastError = what;
}

bool RTMemoryGuard::lockProcessMemory() noexcept
{
#if defined(__linux__)

    if (!checkMemoryLimit(m_heapReserveBytes))
    {
        setError("RLIMIT_MEMLOCK too small", ENOMEM);
        return false;
    }

    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0)
    {
        setError("mlockall", errno);
        std::fprintf(stderr, "RTMemoryGuard: %s\n", m_lastError->c_str());
        return false;
    }

    return true;

#else

    // No mlockall()-equivalent is applied on this platform. Returning true
    // still marks the guard active, so isLocked() will report true even
    // though no real process-wide memory lock was taken here.
    return true;

#endif
}

void RTMemoryGuard::disableMallocPageFaults() noexcept
{
#if defined(__linux__)

    // glibc's mallopt() does not reliably set errno on failure, so errno
    // is not trustworthy here; record a fixed diagnostic instead.
    if (mallopt(M_MMAP_MAX, 0) == 0)
        setError("mallopt(M_MMAP_MAX) failed");

    if (mallopt(M_TRIM_THRESHOLD, -1) == 0)
        setError("mallopt(M_TRIM_THRESHOLD) failed");

#endif
}

void RTMemoryGuard::disableTransparentHugePages() noexcept
{
#if defined(__linux__) && defined(PR_SET_THP_DISABLE)

    if (prctl(PR_SET_THP_DISABLE, 1, 0, 0, 0) != 0) {
        setError("prctl(PR_SET_THP_DISABLE)", errno);
    }

#endif
}

void RTMemoryGuard::requestZeroLatency() noexcept
{
#if defined(__linux__)

    // Avoid reopening (and leaking) the fd if a request is already active.
    if (m_dmaLatencyFd >= 0)
        return;

    m_dmaLatencyFd = open("/dev/cpu_dma_latency", O_RDWR);

    if (m_dmaLatencyFd < 0) {
        return;
}
    const int latency = 0;

    if (write(m_dmaLatencyFd, &latency, sizeof(latency)) < 0) {
        setError("cpu_dma_latency write", errno);
    }
#endif
}

// Must run on the calling RT thread; it only prefaults that thread's own
// stack, not stacks of threads created later.
void RTMemoryGuard::prefaultCurrentThreadStack(std::size_t bytes) noexcept
{
    if (bytes == 0)
        return;

#if defined(__linux__) || defined(__APPLE__)

    touchStackIterative(bytes, queryPageSize());

#elif defined(_WIN32)

    volatile std::uint8_t *page =
        static_cast<volatile std::uint8_t *>(_alloca(bytes));
    page[0] = 0;

#endif
}

std::size_t RTMemoryGuard::queryPageSize() noexcept
{
#if defined(__linux__) || defined(__APPLE__)

    const long size = sysconf(_SC_PAGESIZE);
    if (size > 0)
        return static_cast<std::size_t>(size);

#endif

    return kDefaultPageSize;
}

bool RTMemoryGuard::reserveHeap(std::size_t bytes) noexcept
{
    if (bytes == 0)
        return true;

    const std::size_t pageSize = queryPageSize();
    bytes = (bytes + pageSize - 1) & ~(pageSize - 1);

    void *memory = nullptr;

#if defined(_WIN32)

    memory = VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!memory)
    {
        setError("VirtualAlloc", static_cast<int>(GetLastError()));
        return false;
    }

#else

    const int rc = posix_memalign(&memory, pageSize, bytes);
    if (rc != 0)
    {
        setError("posix_memalign", rc);
        return false;
    }

#endif

    volatile std::uint8_t *ptr = static_cast<volatile std::uint8_t *>(memory);

    for (std::size_t offset = 0; offset < bytes; offset += pageSize)
        ptr[offset] = 0;

    ptr[bytes - 1] = 0;

#if defined(__linux__)

    // mlockall(MCL_FUTURE) is already active, so pages stay locked even
    // after the block is freed back to the glibc arena for reuse.
    std::free(memory);

#elif defined(__APPLE__)

    // No process-wide MCL_FUTURE equivalent is active on this platform,
    // so freeing here does not retain a lock. This only warms the pages.
    std::free(memory);

#elif defined(_WIN32)

    VirtualFree(memory, 0, MEM_RELEASE);

#endif

    return true;
}

void RTMemoryGuard::unlock() noexcept
{
    // Intentionally not gated on m_isActive: activation acquires resources
    // (the latency fd) before m_isActive is set, so cleanup must run even
    // for a partially-completed activation. Each step below is idempotent.

#if defined(__linux__)

    if (m_locked)
    {
        // munlockall() is process-wide; do not run multiple independent
        // RTMemoryGuard instances in the same process.
        munlockall();
    }

    if (m_dmaLatencyFd >= 0)
    {
        close(m_dmaLatencyFd);
        m_dmaLatencyFd = -1;
    }

#endif

    m_locked = false;
    m_heapReserved = false;
    m_isActive = false;
}

} // namespace dinrail
