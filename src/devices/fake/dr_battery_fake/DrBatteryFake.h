// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DRBATTERYFAKE_H
#define DINRAIL_DRBATTERYFAKE_H

#include <dinrail/IBattery.h>
#include <dinrail/IDevice.h>

#include <mutex>
#include <string>

namespace dinrail
{

/**
 * @brief Fake battery device whose measurements can be driven by a simulator.
 */
class DrBatteryFake final : public IDevice, public IBattery, public IBatterySimulation
{
public:
    DrBatteryFake() = default;
    ~DrBatteryFake() override = default;

    bool open(const Parameters& config) override;
    bool close() override;

    Status getBatteryVoltage(double& voltage) override;
    Status getBatteryCurrent(double& current) override;
    Status getBatteryCharge(double& charge) override;
    Status getBatteryStatus(BatteryStatus& status) override;
    Status getBatteryTemperature(double& temperature) override;
    Status getBatteryInfo(std::string& batteryInfo) override;

    Status setBatteryVoltage(double voltage) override;
    Status setBatteryCurrent(double current) override;
    Status setBatteryCharge(double charge) override;
    Status setBatteryStatus(BatteryStatus status) override;
    Status setBatteryTemperature(double temperature) override;
    Status setBatteryInfo(const std::string& batteryInfo) override;

private:
    std::mutex m_mutex;
    bool m_opened{false};
    double m_voltage{0.0};
    double m_current{0.0};
    double m_charge{100.0};
    BatteryStatus m_status{BatteryStatus::OkStandby};
    double m_temperature{25.0};
    std::string m_info{"Fake battery"};
};

} // namespace dinrail

#endif // DINRAIL_DRBATTERYFAKE_H
