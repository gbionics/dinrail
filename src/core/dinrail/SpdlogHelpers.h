/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_SPDLOGHELPERS_H
#define DINRAIL_SPDLOGHELPERS_H

#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

namespace dinrail
{
namespace detail
{

// Loads SPDLOG_LEVEL at most once, on the first successful invocation.
//
// SPDLOG_LEVEL must normally be set before the process starts, or at least
// before the first call to this function. If loading throws, std::call_once
// leaves the flag unset, allowing a later call to retry.
inline void loadSpdlogEnvironmentOnceNoThrow() noexcept
{
    static std::once_flag once;

    try
    {
        std::call_once(once, [] { spdlog::cfg::load_env_levels(); });
    } catch (...)
    {
        // Logging configuration is best-effort.
    }
}

[[nodiscard]]
inline std::shared_ptr<spdlog::logger> defaultLoggerNoThrow() noexcept
{
    try
    {
        return spdlog::default_logger();
    } catch (...)
    {
        return {};
    }
}

inline std::mutex& loggerCreationMutex() noexcept
{
    static std::mutex mutex;
    return mutex;
}

} // namespace detail

[[nodiscard]]
inline std::shared_ptr<spdlog::logger> createOrGetLogger(std::string_view loggerName) noexcept
{
    // Load the environment even when the caller requests the default logger.
    detail::loadSpdlogEnvironmentOnceNoThrow();

    if (loggerName.empty())
    {
        return detail::defaultLoggerNoThrow();
    }

    try
    {
        // Construct explicitly because string_view is not necessarily
        // null-terminated.
        const std::string name{loggerName};

        // Fast path for loggers that already exist.
        if (auto existing = spdlog::get(name))
        {
            return existing;
        }

        // Prevent competing calls through this helper from both trying to
        // register the same logger.
        std::lock_guard<std::mutex> lock{detail::loggerCreationMutex()};

        // Another helper caller may have created it while we waited.
        if (auto existing = spdlog::get(name))
        {
            return existing;
        }

        auto fallback = detail::defaultLoggerNoThrow();
        if (!fallback)
        {
            return {};
        }

        auto candidate = fallback->clone(name);
        if (!candidate)
        {
            // clone() normally either returns a logger or throws, but it is
            // virtual, so defensively handle a null result.
            return fallback;
        }

        // The clone already inherits the default logger's sinks, formatter,
        // flush level, error handler, and other logger configuration.
        //
        // Apply only the level configuration loaded from SPDLOG_LEVEL.
        spdlog::apply_logger_env_levels(candidate);

        try
        {
            // Register explicitly. Do not call initialize_logger(), because
            // it normally registers automatically and may reset the formatter
            // of the sinks shared with the default logger.
            spdlog::register_logger(candidate);
            return candidate;
        } catch (const spdlog::spdlog_ex&)
        {
            // Unrelated code may have registered the same name concurrently.
            if (auto winner = spdlog::get(name))
            {
                return winner;
            }

            // The competing logger may have been dropped immediately after
            // registration. The candidate remains usable, although it is not
            // present in the global registry.
            return candidate;
        }
    } catch (...)
    {
        // Allocation, cloning, locking, environment application, or another
        // unexpected failure.
        return detail::defaultLoggerNoThrow();
    }
}

} // namespace dinrail

// Throttle logging macros
//
// These macros allow logging at most once every `period` seconds.
// Useful for high-frequency events to avoid spamming the logs.
//
// Usage:
//   DINRAIL_WARN_THROTTLE(1.0, my_logger, "Event occurred {} times", count);
//
// Maps to YARP's throttle macros:
//   yWarningThrottle(period, ...) -> DINRAIL_WARN_THROTTLE(period, spdlog::default_logger(), ...)
//   yCWarningThrottle(comp, period, ...) -> DINRAIL_WARN_THROTTLE(period, my_logger, ...)

#define DINRAIL_LOG_THROTTLE_IMPL(level, period, logger, ...)                   \
    do                                                                          \
    {                                                                           \
        static auto last_time = std::chrono::steady_clock::now();               \
        auto now = std::chrono::steady_clock::now();                            \
        if (std::chrono::duration<double>(now - last_time).count() >= (period)) \
        {                                                                       \
            last_time = now;                                                    \
            (logger)->level(__VA_ARGS__);                                       \
        }                                                                       \
    } while (0)

#define DINRAIL_ERROR_THROTTLE(period, logger, ...) \
    DINRAIL_LOG_THROTTLE_IMPL(error, period, logger, __VA_ARGS__)
#define DINRAIL_WARN_THROTTLE(period, logger, ...) \
    DINRAIL_LOG_THROTTLE_IMPL(warn, period, logger, __VA_ARGS__)
#define DINRAIL_INFO_THROTTLE(period, logger, ...) \
    DINRAIL_LOG_THROTTLE_IMPL(info, period, logger, __VA_ARGS__)
#define DINRAIL_DEBUG_THROTTLE(period, logger, ...) \
    DINRAIL_LOG_THROTTLE_IMPL(debug, period, logger, __VA_ARGS__)
#define DINRAIL_TRACE_THROTTLE(period, logger, ...) \
    DINRAIL_LOG_THROTTLE_IMPL(trace, period, logger, __VA_ARGS__)

#endif // DINRAIL_SPDLOGHELPERS_H
