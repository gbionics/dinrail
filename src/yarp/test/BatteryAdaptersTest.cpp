// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>
#include <dinrail/BatteryAdapters.h>
#include <dinrail/Device.h>
#include <dinrail/YarpInteropPlugin.h>

namespace
{
// Expose only the foreign interface so the return trip must use an adapter.
class ForeignBattery final : public dinrail::IDevice, public dinrail::IInterfaceView
{
public:
    explicit ForeignBattery(yarp::dev::IBattery& battery)
        : m_battery(battery)
    {
    }
    bool open(const dinrail::Parameters&) override
    {
        return true;
    }
    bool close() override
    {
        return true;
    }
    void* viewInterface(const std::type_info& type) override
    {
        return type == typeid(yarp::dev::IBattery) ? &m_battery : nullptr;
    }

private:
    yarp::dev::IBattery& m_battery;
};
} // namespace

TEST_CASE("A native battery exposes a cached YARP adapter and round trips through the registry")
{
    dinrail::RuntimeContext context;
    dinrail::Device device(context);
    dinrail::Parameters config;
    config.put("device", "dr_battery_fake");
    config.put("dinrail_device_type", "dinrail");
    REQUIRE(device.open(config));

    dinrail::IBatterySimulation* simulation = nullptr;
    REQUIRE(device.view(simulation));
    REQUIRE(simulation->setBatteryVoltage(48.0).ok());
    REQUIRE(simulation->setBatteryCurrent(2.5).ok());
    REQUIRE(simulation->setBatteryCharge(75.0).ok());
    REQUIRE(simulation->setBatteryTemperature(24.0).ok());
    REQUIRE(simulation->setBatteryInfo("Adapter example battery").ok());

    // Device::view loads the YARP plugin and creates its registered adapter.
    yarp::dev::IBattery* yarpBattery = nullptr;
    REQUIRE(device.view(yarpBattery));
    REQUIRE(yarpBattery != nullptr);
    yarp::dev::IBattery* cached = nullptr;
    REQUIRE(device.view(cached));
    REQUIRE(cached == yarpBattery);

    ForeignBattery foreign(*yarpBattery);
    dinrail::InterfaceAdapterRegistry registry;
    dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(registry);
    auto adapter = registry.create(foreign, typeid(dinrail::IBattery));
    REQUIRE(adapter != nullptr);
    auto* battery = static_cast<dinrail::IBattery*>(adapter->getInterface());
    REQUIRE(battery != nullptr);

    double value = 0;
    REQUIRE(battery->getBatteryVoltage(value).ok());
    REQUIRE(value == 48.0);
    REQUIRE(yarpBattery->getBatteryVoltage(value));
    REQUIRE(value == 48.0);
    REQUIRE(battery->getBatteryCurrent(value).ok());
    REQUIRE(value == 2.5);
    REQUIRE(battery->getBatteryCharge(value).ok());
    REQUIRE(value == 75.0);
    REQUIRE(battery->getBatteryTemperature(value).ok());
    REQUIRE(value == 24.0);
    std::string info;
    REQUIRE(battery->getBatteryInfo(info).ok());
    REQUIRE(info == "Adapter example battery");

    for (int code = 0; code <= 6; ++code)
    {
        const auto expected = static_cast<dinrail::BatteryStatus>(code);
        REQUIRE(simulation->setBatteryStatus(expected).ok());
        dinrail::BatteryStatus status{};
        REQUIRE(battery->getBatteryStatus(status).ok());
        REQUIRE(status == expected);
        yarp::dev::IBattery::Battery_status yarpStatus{};
        REQUIRE(yarpBattery->getBatteryStatus(yarpStatus));
        REQUIRE(static_cast<int>(yarpStatus) == code);
    }

    // Close the source directly to exercise error conversion in both directions
    // while keeping the adapter objects alive.
    dinrail::IDevice* implementation = nullptr;
    REQUIRE(device.view(implementation));
    REQUIRE(implementation->close());
    REQUIRE(yarpBattery->getBatteryVoltage(value)
            == yarp::dev::ReturnValue::return_code::return_value_error_not_ready);
    REQUIRE(battery->getBatteryVoltage(value).code() == dinrail::StatusCode::FailedPrecondition);
    auto status = dinrail::BatteryStatus::LowWarning;
    REQUIRE_FALSE(battery->getBatteryStatus(status).ok());
    REQUIRE(status == dinrail::BatteryStatus::LowWarning);
    auto yarpStatus = static_cast<yarp::dev::IBattery::Battery_status>(5);
    REQUIRE_FALSE(yarpBattery->getBatteryStatus(yarpStatus));
    REQUIRE(static_cast<int>(yarpStatus) == 5);

    adapter.reset(); // Destroy the return adapter before its source.
    REQUIRE(device.close());
    REQUIRE(device.open(config));
    REQUIRE(device.view(yarpBattery));
    REQUIRE(yarpBattery->getBatteryVoltage(value));
    REQUIRE(value == 0.0);
}
