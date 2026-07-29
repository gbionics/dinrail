/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ControlBoardLogComponent.h"

#include <chrono>
#include <dinrail/SpdlogHelpers.h>
#include <mutex>
#include <string>
#include <unordered_map>

spdlog::logger& controlBoardLogger()
{
    static const auto instance = dinrail::createOrGetLogger("dinrail.dr_controlboard_nws_yarp");
    return *instance;
}

bool controlBoardShouldLogThrottle(std::string_view key, double intervalSeconds)
{
    using Clock = std::chrono::steady_clock;

    static std::mutex mutex;
    static std::unordered_map<std::string, Clock::time_point> lastLogTime;

    const auto now = Clock::now();
    const auto interval = std::chrono::duration<double>(intervalSeconds);

    std::lock_guard<std::mutex> lock(mutex);
    auto& last = lastLogTime[std::string(key)];
    if (last.time_since_epoch().count() == 0 || (now - last) >= interval)
    {
        last = now;
        return true;
    }

    return false;
}
