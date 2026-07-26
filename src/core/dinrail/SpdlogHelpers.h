/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DINRAIL_SPDLOGHELPERS_H
#define DINRAIL_SPDLOGHELPERS_H

#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

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

#endif // DINRAIL_SPDLOGHELPERS_H
