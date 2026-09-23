// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "DrBatteryFake.h"

#include <algorithm>
#include <cmath>

namespace dinrail
{

namespace
{

Status deviceNotOpenStatus()
{
    return Status{StatusCode::FailedPrecondition};
}

} // namespace

bool DrBatteryFake::open(const Parameters& config)
{
    static_cast<void>(config);

    std::lock_guard<std::mutex> lock(m_mutex);
    m_voltage = 0.0;
    m_current = 0.0;
    m_charge = 100.0;
    m_status = BatteryStatus::OkStandby;
    m_temperature = 25.0;
    m_info = "Fake battery";
    m_events.clear();
    m_opened = true;
    return true;
}

bool DrBatteryFake::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_opened = false;
    return true;
}

Status DrBatteryFake::getBatteryVoltage(double& voltage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    voltage = m_voltage;
    return OkStatus();
}

Status DrBatteryFake::getBatteryCurrent(double& current)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    current = m_current;
    return OkStatus();
}

Status DrBatteryFake::getBatteryCharge(double& charge)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    charge = m_charge;
    return OkStatus();
}

Status DrBatteryFake::getBatteryStatus(BatteryStatus& status)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    status = m_status;
    return OkStatus();
}

Status DrBatteryFake::getBatteryTemperature(double& temperature)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    temperature = m_temperature;
    return OkStatus();
}

Status DrBatteryFake::getBatteryInfo(std::string& batteryInfo)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    batteryInfo = m_info;
    return OkStatus();
}

Status DrBatteryFake::getBatteryLatestEvents(VectorProxy<BatteryEvent>::Ref events,
                                             std::size_t& eventCount)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    eventCount = std::min(static_cast<std::size_t>(events.size()), m_events.size());
    std::copy_n(m_events.begin(), eventCount, events.begin());
    return OkStatus();
}

Status DrBatteryFake::setBatteryVoltage(double voltage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    m_voltage = voltage;
    return OkStatus();
}

Status DrBatteryFake::setBatteryCurrent(double current)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    m_current = current;
    return OkStatus();
}

Status DrBatteryFake::setBatteryCharge(double charge)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }
    if (!std::isfinite(charge) || charge < 0.0 || charge > 100.0)
    {
        return Status{StatusCode::OutOfRange};
    }

    m_charge = charge;
    return OkStatus();
}

Status DrBatteryFake::setBatteryStatus(BatteryStatus status)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    m_status = status;
    return OkStatus();
}

Status DrBatteryFake::setBatteryTemperature(double temperature)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    m_temperature = temperature;
    return OkStatus();
}

Status DrBatteryFake::setBatteryInfo(const std::string& batteryInfo)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    m_info = batteryInfo;
    return OkStatus();
}

Status DrBatteryFake::setBatteryEvents(const VectorProxy<const BatteryEvent>::Ref events)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return deviceNotOpenStatus();
    }

    m_events.assign(events.begin(), events.end());
    return OkStatus();
}

} // namespace dinrail
