// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>
#include <dinrail/YarpInteropPlugin.h>
#include <dinrail/YarpPreciselyTimedAdapters.h>

#include <chrono>

namespace
{
class NativeTimedDevice final : public dinrail::IDevice, public dinrail::IPreciselyTimed
{
public:
    dinrail::Stamp stamp;
    bool open(const dinrail::Parameters&) override
    {
        return true;
    }
    bool close() override
    {
        return true;
    }
    dinrail::Stamp getLastInputStamp() override
    {
        return stamp;
    }
};

class YarpTimedSource final : public yarp::dev::IPreciselyTimed
{
public:
    yarp::os::Stamp stamp;
    yarp::os::Stamp getLastInputStamp() override
    {
        return stamp;
    }
};

// A foreign device exposes its YARP interface through IInterfaceView, as the
// interop wrapper does, rather than inheriting the interface directly.
class ForeignTimedDevice final : public dinrail::IDevice, public dinrail::IInterfaceView
{
public:
    YarpTimedSource source;
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
        return type == typeid(yarp::dev::IPreciselyTimed)
                   ? static_cast<yarp::dev::IPreciselyTimed*>(&source)
                   : nullptr;
    }
};
} // namespace

TEST_CASE("The registry adapts native timestamps to YARP and observes new samples")
{
    NativeTimedDevice device;
    dinrail::InterfaceAdapterRegistry registry;
    dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(registry);
    auto adapter = registry.create(device, typeid(yarp::dev::IPreciselyTimed));
    REQUIRE(adapter != nullptr);
    auto* timed = static_cast<yarp::dev::IPreciselyTimed*>(adapter->getInterface());
    REQUIRE(timed != nullptr);

    for (const auto& sample : {dinrail::Stamp{},
                               dinrail::Stamp{std::chrono::nanoseconds{1250000000}, 17},
                               dinrail::Stamp{std::chrono::nanoseconds{2750000000}, 23}})
    {
        device.stamp = sample;
        const auto result = timed->getLastInputStamp();
        REQUIRE(result.getCount() == sample.sequenceNumber);
        REQUIRE(result.getTime() == std::chrono::duration<double>(sample.time).count());
    }
}

TEST_CASE("The registry adapts foreign YARP timestamps to dinrail and observes new samples")
{
    ForeignTimedDevice device;
    dinrail::InterfaceAdapterRegistry registry;
    dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(registry);
    auto adapter = registry.create(device, typeid(dinrail::IPreciselyTimed));
    REQUIRE(adapter != nullptr);
    auto* timed = static_cast<dinrail::IPreciselyTimed*>(adapter->getInterface());
    REQUIRE(timed != nullptr);

    // Binary-exact fractional seconds avoid depending on floating-point rounding.
    for (const auto& sample : {dinrail::Stamp{},
                               dinrail::Stamp{std::chrono::nanoseconds{1250000000}, 17},
                               dinrail::Stamp{std::chrono::nanoseconds{2750000000}, 23}})
    {
        device.source.stamp = yarp::os::Stamp(sample.sequenceNumber,
                                              std::chrono::duration<double>(sample.time).count());
        const auto result = timed->getLastInputStamp();
        REQUIRE(result.sequenceNumber == sample.sequenceNumber);
        REQUIRE(result.time == sample.time);
    }
}

TEST_CASE("Timestamp adapters require the corresponding source interface")
{
    NativeTimedDevice native;
    ForeignTimedDevice foreign;
    dinrail::InterfaceAdapterRegistry registry;
    dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(registry);
    REQUIRE(registry.create(native, typeid(dinrail::IPreciselyTimed)) == nullptr);
    REQUIRE(registry.create(foreign, typeid(yarp::dev::IPreciselyTimed)) == nullptr);
}
