// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <FakeMotionControlYarp.h>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <dinrail/Device.h>
#include <dinrail/RuntimeDynamicCast.h>
#include <dinrail/YarpInteropPlugin.h>
#include <vector>
#include <yarp/dev/PolyDriver.h>

namespace
{
// Expose only YARP interfaces, forcing the return trip through real adapters.
class ForeignView final : public dinrail::IDevice, public dinrail::IInterfaceView
{
public:
    explicit ForeignView(yarp::dev::DeviceDriver& source)
        : m_source(source)
    {
    }
    bool open(const dinrail::Parameters&) override
    {
        return false;
    }
    bool close() override
    {
        return false;
    }
    void* viewInterface(const std::type_info& type) override
    {
        return dinrail::runtimeDynamicCast(dinrail::makePolymorphicView(
                                               m_source.getImplementation()),
                                           type);
    }

private:
    yarp::dev::DeviceDriver& m_source;
};

template <class Wrapper> struct NativeDeviceName;

template <class Wrapper> struct RoundTrip
{
    Wrapper wrapper;
    ForeignView foreign{wrapper};
    dinrail::InterfaceAdapterRegistry adapterRegistry;
    std::vector<std::unique_ptr<dinrail::IInterfaceAdapter>> adapters;
    RoundTrip()
    {
        yarp::os::Property config;
        config.put("device", NativeDeviceName<Wrapper>::value);
        config.put("number_of_joints", 3);
        REQUIRE(wrapper.open(config));
        dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(adapterRegistry);
    }
    ~RoundTrip()
    {
        wrapper.close();
    }
    template <class Interface> Interface& view()
    {
        auto adapter = adapterRegistry.create(foreign, typeid(Interface));
        REQUIRE(adapter != nullptr);
        auto* result = static_cast<Interface*>(adapter->getInterface());
        REQUIRE(result != nullptr);
        adapters.push_back(std::move(adapter));
        return *result;
    }
    template <class Interface> Interface& reverse()
    {
        Interface* result = nullptr;
        REQUIRE(wrapper.device().view(result));
        return *result;
    }
};

template <> struct NativeDeviceName<FakeMotionControlYarp>
{
    static constexpr const char* value = "dr_controlboard_fake";
};

template <class Interface, class Wrapper> Interface& viewNative(Wrapper& wrapper)
{
    Interface* result = nullptr;
    REQUIRE(wrapper.device().view(result));
    return *result;
}
} // namespace

TEST_CASE("Control board measurements and supported setters survive a round trip")
{
    RoundTrip<FakeMotionControlYarp> trip;
    auto& axes = trip.view<dinrail::IAxisInfo>();
    auto& axesReverse = trip.reverse<yarp::dev::IAxisInfo>();
    int count = 0;
    REQUIRE(axes.getAxes(&count));
    REQUIRE(count == 3);
    std::string name, expectedName;
    REQUIRE(viewNative<dinrail::IAxisInfo>(trip.wrapper).getAxisName(1, expectedName));
    REQUIRE(axes.getAxisName(1, name));
    REQUIRE(name == expectedName);
    REQUIRE(axesReverse.getAxisName(1, name));
    REQUIRE(name == expectedName);
    dinrail::JointType joint, expectedJoint;
    REQUIRE(viewNative<dinrail::IAxisInfo>(trip.wrapper).getJointType(1, expectedJoint));
    REQUIRE(axes.getJointType(1, joint));
    REQUIRE(joint == expectedJoint);
    REQUIRE_FALSE(axes.getAxisName(3, name));
    const std::vector<double> values{1.25, -2.5, 3.75};
    const std::vector<double> times{10.0, 20.0, 30.0};
    const std::vector<double> speeds{4.0, 5.0, 6.0};
    const std::vector<double> accelerations{7.0, 8.0, 9.0};
    {
        auto& encoders = trip.view<dinrail::IEncoders>();
        auto& source = viewNative<dinrail::IEncodersSimulation>(trip.wrapper);
        REQUIRE(source.setEncodersTimed(values, times));
        REQUIRE(source.setEncoderSpeeds(speeds));
        REQUIRE(source.setEncoderAccelerations(accelerations));
        std::vector<double> output, stamps;
        double value = 0, timestamp = 0;
        REQUIRE(encoders.getEncoders(output));
        REQUIRE(output == values);
        REQUIRE(encoders.getEncoder(1, &value));
        REQUIRE(value == values[1]);
        REQUIRE(encoders.getEncoderSpeeds(output));
        REQUIRE(output == speeds);
        REQUIRE(encoders.getEncoderSpeed(1, &value));
        REQUIRE(value == speeds[1]);
        REQUIRE(encoders.getEncoderAccelerations(output));
        REQUIRE(output == accelerations);
        REQUIRE(encoders.getEncoderAcceleration(1, &value));
        REQUIRE(value == accelerations[1]);
        REQUIRE(encoders.getEncodersTimed(output, stamps));
        REQUIRE(output == values);
        REQUIRE(stamps == times);
        REQUIRE(encoders.getEncoderTimed(2, &value, &timestamp));
        REQUIRE(value == values[2]);
        REQUIRE(timestamp == times[2]);
        REQUIRE_FALSE(encoders.getEncoder(3, &value));
        REQUIRE_FALSE(encoders.getEncoder(0, nullptr));
        std::array<double, 2> shortOutput{};
        REQUIRE_FALSE(encoders.getEncoders(shortOutput));
        auto& reverse = trip.reverse<yarp::dev::IEncodersTimed>();
        REQUIRE_FALSE(reverse.resetEncoder(0));
        REQUIRE_FALSE(reverse.resetEncoders());
        REQUIRE_FALSE(reverse.setEncoder(0, 0.0));
        REQUIRE_FALSE(reverse.setEncoders(values.data()));
        std::array<double, 5> guarded{999, 0, 0, 0, 999};
        REQUIRE(reverse.getEncoders(guarded.data() + 1));
        REQUIRE(guarded.front() == 999);
        REQUIRE(guarded.back() == 999);
        REQUIRE(guarded[2] == values[1]);
        REQUIRE_FALSE(reverse.getEncoders(nullptr));
        REQUIRE(encoders.getAxes(&count));
        REQUIRE(count == 3);
        auto& plain = trip.reverse<yarp::dev::IEncoders>();
        REQUIRE(plain.getEncoder(1, &value));
        REQUIRE(value == values[1]);
    }
    {
        auto& encoders = trip.view<dinrail::IMotorEncoders>();
        auto& source = viewNative<dinrail::IMotorEncodersSimulation>(trip.wrapper);
        REQUIRE(source.setMotorEncodersTimed(values, times));
        REQUIRE(source.setMotorEncoderSpeeds(speeds));
        REQUIRE(source.setMotorEncoderAccelerations(accelerations));
        std::vector<double> output, stamps;
        double value = 0, timestamp = 0;
        REQUIRE(encoders.getMotorEncoders(output));
        REQUIRE(output == values);
        REQUIRE(encoders.getMotorEncoder(1, &value));
        REQUIRE(value == values[1]);
        REQUIRE(encoders.getMotorEncoderSpeeds(output));
        REQUIRE(output == speeds);
        REQUIRE(encoders.getMotorEncoderSpeed(1, &value));
        REQUIRE(value == speeds[1]);
        REQUIRE(encoders.getMotorEncoderAccelerations(output));
        REQUIRE(output == accelerations);
        REQUIRE(encoders.getMotorEncoderAcceleration(1, &value));
        REQUIRE(value == accelerations[1]);
        REQUIRE(encoders.getMotorEncodersTimed(output, stamps));
        REQUIRE(output == values);
        REQUIRE(stamps == times);
        REQUIRE(encoders.getMotorEncoderTimed(2, &value, &timestamp));
        REQUIRE(value == values[2]);
        REQUIRE(timestamp == times[2]);
        REQUIRE_FALSE(encoders.getMotorEncoder(3, &value));
        REQUIRE_FALSE(encoders.getMotorEncoder(0, nullptr));
        std::array<double, 2> shortOutput{};
        REQUIRE_FALSE(encoders.getMotorEncoders(shortOutput));
        auto& reverse = trip.reverse<yarp::dev::IMotorEncoders>();
        REQUIRE_FALSE(reverse.resetMotorEncoder(0));
        REQUIRE_FALSE(reverse.resetMotorEncoders());
        REQUIRE_FALSE(reverse.setMotorEncoder(0, 0.0));
        REQUIRE_FALSE(reverse.setMotorEncoders(values.data()));
        std::array<double, 5> guarded{999, 0, 0, 0, 999};
        REQUIRE(reverse.getMotorEncoders(guarded.data() + 1));
        REQUIRE(guarded.front() == 999);
        REQUIRE(guarded.back() == 999);
        REQUIRE(guarded[2] == values[1]);
        REQUIRE_FALSE(reverse.getMotorEncoders(nullptr));
        REQUIRE_FALSE(reverse.setMotorEncoderCountsPerRevolution(0, 42));
        double expected = 0;
        REQUIRE(viewNative<dinrail::IMotorEncoders>(trip.wrapper)
                    .getMotorEncoderCountsPerRevolution(0, &expected));
        REQUIRE(encoders.getMotorEncoderCountsPerRevolution(0, &value));
        REQUIRE(value == expected);
        REQUIRE(encoders.getNumberOfMotorEncoders(&count));
        REQUIRE(count == 3);
    }
    auto& motor = trip.view<dinrail::IMotor>();
    auto& reverseMotor = trip.reverse<yarp::dev::IMotor>();
    auto& motorSource = viewNative<dinrail::IMotorSimulation>(trip.wrapper);
    REQUIRE(motorSource.setTemperatures(values));
    std::vector<double> output;
    REQUIRE(motor.getNumberOfMotors(&count));
    REQUIRE(count == 3);
    REQUIRE(motor.getTemperatures(output));
    REQUIRE(output == values);
    double value = 0;
    REQUIRE(motor.getTemperature(1, &value));
    REQUIRE(value == values[1]);
    REQUIRE(motor.setTemperatureLimit(1, 80));
    REQUIRE(motor.getTemperatureLimit(1, &value));
    REQUIRE(value == 80);
    REQUIRE(viewNative<dinrail::IMotor>(trip.wrapper).getTemperatureLimit(1, &value));
    REQUIRE(value == 80);
    REQUIRE(motor.setGearboxRatio(1, 42));
    REQUIRE(motor.getGearboxRatio(1, &value));
    REQUIRE(value == 42);
    REQUIRE(reverseMotor.getGearboxRatio(1, &value));
    REQUIRE(value == 42);
    REQUIRE_FALSE(reverseMotor.getTemperatures(nullptr));
    auto& fault = trip.view<dinrail::IJointFault>();
    REQUIRE(viewNative<dinrail::IJointFaultSimulation>(trip.wrapper)
                .setLastJointFault(1, 42, "test fault"));
    int code = 0;
    REQUIRE(fault.getLastJointFault(1, code, name));
    REQUIRE(code == 42);
    REQUIRE(name == "test fault");
    auto& reverseFault = trip.reverse<yarp::dev::IJointFault>();
    REQUIRE(reverseFault.getLastJointFault(1, code, name));
    REQUIRE(code == 42);
    auto& timed = trip.view<dinrail::IPreciselyTimed>();
    const dinrail::Stamp stamp{std::chrono::nanoseconds{1250000000}, 123};
    viewNative<dinrail::IPreciselyTimedSimulation>(trip.wrapper).setLastInputStamp(stamp);
    REQUIRE(timed.getLastInputStamp().time == stamp.time);
    REQUIRE(timed.getLastInputStamp().sequenceNumber == stamp.sequenceNumber);
    auto& reverseTimed = trip.reverse<yarp::dev::IPreciselyTimed>();
    REQUIRE(reverseTimed.getLastInputStamp().getCount() == 123);
    REQUIRE(reverseTimed.getLastInputStamp().getTime() == 1.25);
}

TEST_CASE("Explicit YARP interface lists restrict exposure and share inherited interfaces")
{
    dinrail::YarpDeviceFromDinrail<
        dinrail::InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>>
        wrapper;
    yarp::os::Property config;
    config.put("device", "dr_controlboard_fake");
    config.put("number_of_joints", 3);
    REQUIRE(wrapper.open(config));
    yarp::dev::IEncoders* encoders = nullptr;
    yarp::dev::IEncodersTimed* timed = nullptr;
    REQUIRE(wrapper.view(encoders));
    REQUIRE(wrapper.view(timed));
    REQUIRE(encoders == static_cast<yarp::dev::IEncoders*>(timed));
    yarp::dev::IMotor* unrequested = nullptr;
    REQUIRE_FALSE(wrapper.view(unrequested));
    int axes = 0;
    REQUIRE(encoders->getAxes(&axes));
    REQUIRE(axes == 3);
    REQUIRE(wrapper.close());
}

TEST_CASE("A selected adapter prevents opening when its source is unavailable")
{
    dinrail::YarpDeviceFromDinrail<
        dinrail::InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>>
        wrapper;
    yarp::os::Property config;
    config.put("device", "dr_battery_fake");
    REQUIRE_FALSE(wrapper.open(config));
    REQUIRE_FALSE(wrapper.device().isValid());
}

TEST_CASE("Same-interface impedance forwarding preserves all setters and getters")
{
    FakeMotionControlYarp wrapper;
    yarp::os::Property config;
    config.put("device", "dr_controlboard_fake");
    config.put("number_of_joints", 3);
    REQUIRE(wrapper.open(config));
    dinrail::IImpedanceAllSetPointsControl* shared = nullptr;
    REQUIRE(wrapper.view(shared));
    const std::vector<double> pos{1, 2, 3}, vel{4, 5, 6}, torque{7, 8, 9}, stiffness{10, 11, 12},
        damping{13, 14, 15};
    REQUIRE(shared->setSetPoints(pos, vel, torque, stiffness, damping));
    std::vector<double> p, v, t, k, d;
    p.resize(3);
    v.resize(3);
    t.resize(3);
    k.resize(3);
    d.resize(3);
    REQUIRE(shared->getSetPoints(p, v, t, k, d));
    REQUIRE(p == pos);
    const std::vector<int> joints{2, 0};
    const std::vector<double> values{42, 43};
    REQUIRE(shared->setSetPoints(joints, values, values, values, values, values));
    p.resize(2);
    v.resize(2);
    t.resize(2);
    k.resize(2);
    d.resize(2);
    REQUIRE(shared->getSetPoints(joints, p, v, t, k, d));
    REQUIRE(p == values);
    REQUIRE(v == values);
    REQUIRE(t == values);
    REQUIRE(k == values);
    REQUIRE(d == values);
    double a = 0, b = 0, c = 0, e = 0, f = 0;
    REQUIRE(shared->getSetPoint(2, a, b, c, e, f));
    REQUIRE(a == 42);
    REQUIRE_FALSE(shared->getSetPoint(3, a, b, c, e, f));
    REQUIRE_FALSE(shared->setSetPoint(3, 1, 2, 3, 4, 5));
    REQUIRE(wrapper.close());
}

TEST_CASE("An encoder-only YARP source adapts without fabricating timestamps")
{
    dinrail::YarpDeviceFromDinrail<
        dinrail::InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>>
        wrapper;
    yarp::os::Property config;
    config.put("device", "dr_controlboard_fake");
    config.put("number_of_joints", 3);
    REQUIRE(wrapper.open(config));
    auto& simulation = viewNative<dinrail::IEncodersSimulation>(wrapper);
    const std::vector<double> expected{1, 2, 3};
    REQUIRE(simulation.setEncoders(expected));
    ForeignView foreign(wrapper);
    dinrail::InterfaceAdapterRegistry registry;
    dinrail::YarpInteropPlugin{}.registerInterfaceAdapters(registry);
    auto adapter = registry.create(foreign, typeid(dinrail::IEncoders));
    REQUIRE(adapter != nullptr);
    auto* encoders = static_cast<dinrail::IEncoders*>(adapter->getInterface());
    std::vector<double> values, timestamps;
    REQUIRE(encoders->getEncoders(values));
    REQUIRE(values == expected);
    REQUIRE_FALSE(encoders->getEncodersTimed(values, timestamps));
    double value = 0, timestamp = 0;
    REQUIRE_FALSE(encoders->getEncoderTimed(0, &value, &timestamp));
    REQUIRE(wrapper.close());
}

TEST_CASE("A native controlboard exposes a cached YARP encoder adapter")
{
    dinrail::Device device;
    dinrail::Parameters config;
    config.put("device", "dr_controlboard_fake");
    config.put("dinrail_device_type", "dinrail");
    config.put("number_of_joints", 3);
    REQUIRE(device.open(config));
    dinrail::IEncodersSimulation* simulation = nullptr;
    REQUIRE(device.view(simulation));
    REQUIRE(simulation->setEncoderTimed(1, 42.5, 1.25));
    yarp::dev::IEncodersTimed* adapted = nullptr;
    REQUIRE(device.view(adapted));
    yarp::dev::IEncodersTimed* cached = nullptr;
    REQUIRE(device.view(cached));
    REQUIRE(cached == adapted);
    double value = 0, timestamp = 0;
    REQUIRE(adapted->getEncoderTimed(1, &value, &timestamp));
    REQUIRE(value == 42.5);
    REQUIRE(timestamp == 1.25);
    REQUIRE(device.close());
}

#ifdef DINRAIL_HAS_YARP_CONTROLBOARD_WRAPPER
TEST_CASE("The controlboard fake loads as an installed YARP plugin")
{
    yarp::os::Property config;
    config.put("device", "dr_controlboard_fake");
    config.put("number_of_joints", 3);
    yarp::dev::PolyDriver driver;
    REQUIRE(driver.open(config));
    yarp::dev::IAxisInfo* axes = nullptr;
    yarp::dev::IEncodersTimed* timed = nullptr;
    yarp::dev::IEncoders* encoders = nullptr;
    yarp::dev::IMotor* motor = nullptr;
    yarp::dev::IMotorEncoders* motorEncoders = nullptr;
    yarp::dev::IJointFault* fault = nullptr;
    yarp::dev::IPreciselyTimed* stamp = nullptr;
    dinrail::IImpedanceAllSetPointsControl* impedance = nullptr;
    REQUIRE(driver.view(axes));
    REQUIRE(driver.view(timed));
    REQUIRE(driver.view(encoders));
    REQUIRE(driver.view(motor));
    REQUIRE(driver.view(motorEncoders));
    REQUIRE(driver.view(fault));
    REQUIRE(driver.view(stamp));
    REQUIRE(driver.view(impedance));
    REQUIRE(encoders == static_cast<yarp::dev::IEncoders*>(timed));
    int count = 0;
    REQUIRE(encoders->getAxes(&count));
    REQUIRE(count == 3);
    REQUIRE(driver.close());
}
#endif
