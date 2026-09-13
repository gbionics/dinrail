// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <DrBatteryFakeYarp.h>
#include <DrMultipleAnalogSensorsFakeYarp.h>
#include <FakeMotionControlYarp.h>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <dinrail/Device.h>
#include <dinrail/IJoypadControl.h>
#include <dinrail/RuntimeDynamicCast.h>
#include <dinrail/YarpDeviceFromDinrail.h>
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

template <> struct NativeDeviceName<DrBatteryFakeYarp>
{
    static constexpr const char* value = "dr_battery_fake";
};
template <> struct NativeDeviceName<FakeMotionControlYarp>
{
    static constexpr const char* value = "dr_controlboard_fake";
};
template <> struct NativeDeviceName<DrMultipleAnalogSensorsFakeYarp>
{
    static constexpr const char* value = "dr_multiplenalogsensors_fake";
};

template <class Interface, class Wrapper> Interface& viewNative(Wrapper& wrapper)
{
    Interface* result = nullptr;
    REQUIRE(wrapper.device().view(result));
    return *result;
}
} // namespace

TEST_CASE("Battery measurements survive dinrail to YARP to dinrail adaptation")
{
    RoundTrip<DrBatteryFakeYarp> trip;
    auto& source = viewNative<dinrail::IBatterySimulation>(trip.wrapper);
    auto& battery = trip.view<dinrail::IBattery>();
    auto& reverse = trip.reverse<yarp::dev::IBattery>();
    REQUIRE(source.setBatteryVoltage(48.0).ok());
    REQUIRE(source.setBatteryCurrent(2.5).ok());
    REQUIRE(source.setBatteryCharge(75.0).ok());
    REQUIRE(source.setBatteryTemperature(24.0).ok());
    REQUIRE(source.setBatteryInfo("Round trip battery").ok());
    REQUIRE(source.setBatteryStatus(dinrail::BatteryStatus::OkInUse).ok());
    double value = 0;
    REQUIRE(battery.getBatteryVoltage(value).ok());
    REQUIRE(value == 48.0);
    REQUIRE(reverse.getBatteryVoltage(value));
    REQUIRE(value == 48.0);
    REQUIRE(battery.getBatteryCurrent(value).ok());
    REQUIRE(value == 2.5);
    REQUIRE(battery.getBatteryCharge(value).ok());
    REQUIRE(value == 75.0);
    REQUIRE(battery.getBatteryTemperature(value).ok());
    REQUIRE(value == 24.0);
    std::string info;
    REQUIRE(battery.getBatteryInfo(info).ok());
    REQUIRE(info == "Round trip battery");
    dinrail::BatteryStatus status{};
    REQUIRE(battery.getBatteryStatus(status).ok());
    REQUIRE(status == dinrail::BatteryStatus::OkInUse);
    yarp::dev::IEncoders* unrelated = nullptr;
    REQUIRE_FALSE(trip.wrapper.view(unrelated));
    yarp::dev::IEncoders* unavailable = nullptr;
    REQUIRE_FALSE(trip.wrapper.device().view(unavailable));
    REQUIRE(trip.wrapper.close());
    yarp::os::Property config;
    config.put("device", "dr_battery_fake");
    REQUIRE(trip.wrapper.open(config));
    REQUIRE(source.setBatteryVoltage(12.0).ok());
    REQUIRE(battery.getBatteryVoltage(value).ok());
    REQUIRE(value == 12.0);
}

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

TEST_CASE("dinrail::IThreeAxisGyroscopes survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::IThreeAxisGyroscopesSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::IThreeAxisGyroscopes>();
    auto& reverse = trip.reverse<yarp::dev::IThreeAxisGyroscopes>();
    const std::vector<double> values(3, 12.5);
    REQUIRE(native.setThreeAxisGyroscopeMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfThreeAxisGyroscopes() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getThreeAxisGyroscopeMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getThreeAxisGyroscopeMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 3);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::IThreeAxisGyroscopes>(trip.wrapper)
                .getThreeAxisGyroscopeName(0, expected));
    REQUIRE(sensor.getThreeAxisGyroscopeName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(viewNative<dinrail::IThreeAxisGyroscopes>(trip.wrapper)
                .getThreeAxisGyroscopeFrameName(0, expected));
    REQUIRE(sensor.getThreeAxisGyroscopeFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setThreeAxisGyroscopeStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getThreeAxisGyroscopeStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getThreeAxisGyroscopeMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getThreeAxisGyroscopeMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getThreeAxisGyroscopeMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::IThreeAxisLinearAccelerometers survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::IThreeAxisLinearAccelerometersSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::IThreeAxisLinearAccelerometers>();
    auto& reverse = trip.reverse<yarp::dev::IThreeAxisLinearAccelerometers>();
    const std::vector<double> values(3, 12.5);
    REQUIRE(native.setThreeAxisLinearAccelerometerMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfThreeAxisLinearAccelerometers() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getThreeAxisLinearAccelerometerMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getThreeAxisLinearAccelerometerMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 3);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::IThreeAxisLinearAccelerometers>(trip.wrapper)
                .getThreeAxisLinearAccelerometerName(0, expected));
    REQUIRE(sensor.getThreeAxisLinearAccelerometerName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(viewNative<dinrail::IThreeAxisLinearAccelerometers>(trip.wrapper)
                .getThreeAxisLinearAccelerometerFrameName(0, expected));
    REQUIRE(sensor.getThreeAxisLinearAccelerometerFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setThreeAxisLinearAccelerometerStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getThreeAxisLinearAccelerometerStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getThreeAxisLinearAccelerometerMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getThreeAxisLinearAccelerometerMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getThreeAxisLinearAccelerometerMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::IThreeAxisAngularAccelerometers survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::IThreeAxisAngularAccelerometersSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::IThreeAxisAngularAccelerometers>();
    auto& reverse = trip.reverse<yarp::dev::IThreeAxisAngularAccelerometers>();
    const std::vector<double> values(3, 12.5);
    REQUIRE(native.setThreeAxisAngularAccelerometerMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfThreeAxisAngularAccelerometers() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getThreeAxisAngularAccelerometerMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getThreeAxisAngularAccelerometerMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 3);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::IThreeAxisAngularAccelerometers>(trip.wrapper)
                .getThreeAxisAngularAccelerometerName(0, expected));
    REQUIRE(sensor.getThreeAxisAngularAccelerometerName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(viewNative<dinrail::IThreeAxisAngularAccelerometers>(trip.wrapper)
                .getThreeAxisAngularAccelerometerFrameName(0, expected));
    REQUIRE(sensor.getThreeAxisAngularAccelerometerFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setThreeAxisAngularAccelerometerStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getThreeAxisAngularAccelerometerStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getThreeAxisAngularAccelerometerMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getThreeAxisAngularAccelerometerMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getThreeAxisAngularAccelerometerMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::IThreeAxisMagnetometers survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::IThreeAxisMagnetometersSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::IThreeAxisMagnetometers>();
    auto& reverse = trip.reverse<yarp::dev::IThreeAxisMagnetometers>();
    const std::vector<double> values(3, 12.5);
    REQUIRE(native.setThreeAxisMagnetometerMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfThreeAxisMagnetometers() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getThreeAxisMagnetometerMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getThreeAxisMagnetometerMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 3);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::IThreeAxisMagnetometers>(trip.wrapper)
                .getThreeAxisMagnetometerName(0, expected));
    REQUIRE(sensor.getThreeAxisMagnetometerName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(viewNative<dinrail::IThreeAxisMagnetometers>(trip.wrapper)
                .getThreeAxisMagnetometerFrameName(0, expected));
    REQUIRE(sensor.getThreeAxisMagnetometerFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setThreeAxisMagnetometerStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getThreeAxisMagnetometerStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getThreeAxisMagnetometerMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getThreeAxisMagnetometerMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getThreeAxisMagnetometerMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::IPositionSensors survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::IPositionSensorsSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::IPositionSensors>();
    auto& reverse = trip.reverse<yarp::dev::IPositionSensors>();
    const std::vector<double> values(3, 12.5);
    REQUIRE(native.setPositionSensorMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfPositionSensors() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getPositionSensorMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getPositionSensorMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 3);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::IPositionSensors>(trip.wrapper).getPositionSensorName(0, expected));
    REQUIRE(sensor.getPositionSensorName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(
        viewNative<dinrail::IPositionSensors>(trip.wrapper).getPositionSensorFrameName(0, expected));
    REQUIRE(sensor.getPositionSensorFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setPositionSensorStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getPositionSensorStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getPositionSensorMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getPositionSensorMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getPositionSensorMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::ILinearVelocitySensors survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::ILinearVelocitySensorsSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::ILinearVelocitySensors>();
    auto& reverse = trip.reverse<yarp::dev::ILinearVelocitySensors>();
    const std::vector<double> values(3, 12.5);
    REQUIRE(native.setLinearVelocitySensorMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfLinearVelocitySensors() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getLinearVelocitySensorMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getLinearVelocitySensorMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 3);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::ILinearVelocitySensors>(trip.wrapper)
                .getLinearVelocitySensorName(0, expected));
    REQUIRE(sensor.getLinearVelocitySensorName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(viewNative<dinrail::ILinearVelocitySensors>(trip.wrapper)
                .getLinearVelocitySensorFrameName(0, expected));
    REQUIRE(sensor.getLinearVelocitySensorFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setLinearVelocitySensorStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getLinearVelocitySensorStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getLinearVelocitySensorMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getLinearVelocitySensorMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getLinearVelocitySensorMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::IOrientationSensors survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::IOrientationSensorsSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::IOrientationSensors>();
    auto& reverse = trip.reverse<yarp::dev::IOrientationSensors>();
    const std::vector<double> values(3, 12.5);
    REQUIRE(native.setOrientationSensorMeasureAsRollPitchYaw(0, values, 42.25));
    REQUIRE(sensor.getNrOfOrientationSensors() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getOrientationSensorMeasureAsRollPitchYaw(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getOrientationSensorMeasureAsRollPitchYaw(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 3);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::IOrientationSensors>(trip.wrapper)
                .getOrientationSensorName(0, expected));
    REQUIRE(sensor.getOrientationSensorName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(viewNative<dinrail::IOrientationSensors>(trip.wrapper)
                .getOrientationSensorFrameName(0, expected));
    REQUIRE(sensor.getOrientationSensorFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setOrientationSensorStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getOrientationSensorStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getOrientationSensorMeasureAsRollPitchYaw(1, output, timestamp));
    REQUIRE_FALSE(reverse.getOrientationSensorMeasureAsRollPitchYaw(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getOrientationSensorMeasureAsRollPitchYaw(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::ISixAxisForceTorqueSensors survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::ISixAxisForceTorqueSensorsSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::ISixAxisForceTorqueSensors>();
    auto& reverse = trip.reverse<yarp::dev::ISixAxisForceTorqueSensors>();
    const std::vector<double> values(6, 12.5);
    REQUIRE(native.setSixAxisForceTorqueSensorMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfSixAxisForceTorqueSensors() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getSixAxisForceTorqueSensorMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getSixAxisForceTorqueSensorMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 6);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::ISixAxisForceTorqueSensors>(trip.wrapper)
                .getSixAxisForceTorqueSensorName(0, expected));
    REQUIRE(sensor.getSixAxisForceTorqueSensorName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(viewNative<dinrail::ISixAxisForceTorqueSensors>(trip.wrapper)
                .getSixAxisForceTorqueSensorFrameName(0, expected));
    REQUIRE(sensor.getSixAxisForceTorqueSensorFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setSixAxisForceTorqueSensorStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getSixAxisForceTorqueSensorStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getSixAxisForceTorqueSensorMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getSixAxisForceTorqueSensorMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getSixAxisForceTorqueSensorMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::IContactLoadCellArrays survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::IContactLoadCellArraysSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::IContactLoadCellArrays>();
    auto& reverse = trip.reverse<yarp::dev::IContactLoadCellArrays>();
    const std::vector<double> values(4, 12.5);
    REQUIRE(native.setContactLoadCellArrayMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfContactLoadCellArrays() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getContactLoadCellArrayMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getContactLoadCellArrayMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 4);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::IContactLoadCellArrays>(trip.wrapper)
                .getContactLoadCellArrayName(0, expected));
    REQUIRE(sensor.getContactLoadCellArrayName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(sensor.getContactLoadCellArraySize(0) == 4);
    REQUIRE(native.setContactLoadCellArrayStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getContactLoadCellArrayStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getContactLoadCellArrayMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getContactLoadCellArrayMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getContactLoadCellArrayMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::IEncoderArrays survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::IEncoderArraysSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::IEncoderArrays>();
    auto& reverse = trip.reverse<yarp::dev::IEncoderArrays>();
    const std::vector<double> values(4, 12.5);
    REQUIRE(native.setEncoderArrayMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfEncoderArrays() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getEncoderArrayMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getEncoderArrayMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 4);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::IEncoderArrays>(trip.wrapper).getEncoderArrayName(0, expected));
    REQUIRE(sensor.getEncoderArrayName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(sensor.getEncoderArraySize(0) == 4);
    REQUIRE(native.setEncoderArrayStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getEncoderArrayStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getEncoderArrayMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getEncoderArrayMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getEncoderArrayMeasure(0, shortOutput, timestamp));
}

TEST_CASE("dinrail::ISkinPatches survives a dinrail YARP round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::ISkinPatchesSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::ISkinPatches>();
    auto& reverse = trip.reverse<yarp::dev::ISkinPatches>();
    const std::vector<double> values(4, 12.5);
    REQUIRE(native.setSkinPatchMeasure(0, values, 42.25));
    REQUIRE(sensor.getNrOfSkinPatches() == 1);
    std::vector<double> output;
    double timestamp = 0;
    REQUIRE(sensor.getSkinPatchMeasure(0, output, timestamp));
    REQUIRE(output == values);
    REQUIRE(timestamp == 42.25);
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getSkinPatchMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 4);
    REQUIRE(yarpOutput[0] == 12.5);
    REQUIRE(timestamp == 42.25);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::ISkinPatches>(trip.wrapper).getSkinPatchName(0, expected));
    REQUIRE(sensor.getSkinPatchName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(sensor.getSkinPatchSize(0) == 4);
    REQUIRE(native.setSkinPatchStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getSkinPatchStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getSkinPatchMeasure(1, output, timestamp));
    REQUIRE_FALSE(reverse.getSkinPatchMeasure(1, yarpOutput, timestamp));
    std::array<double, 1> shortOutput{};
    REQUIRE_FALSE(sensor.getSkinPatchMeasure(0, shortOutput, timestamp));
}

TEST_CASE("Temperature sensor scalar and vector overloads survive a round trip")
{
    RoundTrip<DrMultipleAnalogSensorsFakeYarp> trip;
    auto& native = viewNative<dinrail::ITemperatureSensorsSimulation>(trip.wrapper);
    auto& sensor = trip.view<dinrail::ITemperatureSensors>();
    auto& reverse = trip.reverse<yarp::dev::ITemperatureSensors>();
    REQUIRE(native.setTemperatureSensorMeasure(0, 37.5, 42.25));
    REQUIRE(sensor.getNrOfTemperatureSensors() == 1);
    double value = 0, timestamp = 0;
    REQUIRE(sensor.getTemperatureSensorMeasure(0, value, timestamp));
    REQUIRE(value == 37.5);
    REQUIRE(timestamp == 42.25);
    std::vector<double> output;
    REQUIRE(sensor.getTemperatureSensorMeasure(0, output, timestamp));
    REQUIRE(output == std::vector<double>{37.5});
    yarp::sig::Vector yarpOutput;
    REQUIRE(reverse.getTemperatureSensorMeasure(0, yarpOutput, timestamp));
    REQUIRE(yarpOutput.size() == 1);
    REQUIRE(yarpOutput[0] == 37.5);
    std::string expected, actual;
    REQUIRE(viewNative<dinrail::ITemperatureSensors>(trip.wrapper)
                .getTemperatureSensorName(0, expected));
    REQUIRE(sensor.getTemperatureSensorName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(viewNative<dinrail::ITemperatureSensors>(trip.wrapper)
                .getTemperatureSensorFrameName(0, expected));
    REQUIRE(sensor.getTemperatureSensorFrameName(0, actual));
    REQUIRE(actual == expected);
    REQUIRE(native.setTemperatureSensorStatus(0, dinrail::MAS_status::MAS_ERROR));
    REQUIRE(sensor.getTemperatureSensorStatus(0) == dinrail::MAS_status::MAS_ERROR);
    REQUIRE_FALSE(sensor.getTemperatureSensorMeasure(1, value, timestamp));
}

TEST_CASE("Every native device loads as an installed YARP plugin")
{
    for (const auto* name :
         {"dr_battery_fake", "dr_controlboard_fake", "dr_multiplenalogsensors_fake"})
    {
        CAPTURE(name);
        yarp::os::Property config;
        config.put("device", name);
        config.put("number_of_joints", 3);
        yarp::dev::PolyDriver driver;
        REQUIRE(driver.open(config));
        if (std::string(name) == "dr_battery_fake")
        {
            yarp::dev::IBattery* battery = nullptr;
            REQUIRE(driver.view(battery));
            double voltage = 0;
            REQUIRE(battery->getBatteryVoltage(voltage));
            yarp::dev::IEncoders* unsupported = nullptr;
            REQUIRE_FALSE(driver.view(unsupported));
        } else if (std::string(name) == "dr_controlboard_fake")
        {
            yarp::dev::IEncoders* encoders = nullptr;
            yarp::dev::IEncodersTimed* timed = nullptr;
            REQUIRE(driver.view(encoders));
            REQUIRE(driver.view(timed));
            int axes = 0;
            REQUIRE(encoders->getAxes(&axes));
            REQUIRE(axes == 3);
            yarp::dev::IAxisInfo* info = nullptr;
            REQUIRE(driver.view(info));
            yarp::dev::IMotor* motor = nullptr;
            REQUIRE(driver.view(motor));
            yarp::dev::IMotorEncoders* motorEncoders = nullptr;
            REQUIRE(driver.view(motorEncoders));
            yarp::dev::IJointFault* fault = nullptr;
            REQUIRE(driver.view(fault));
            dinrail::IImpedanceAllSetPointsControl* impedance = nullptr;
            REQUIRE(driver.view(impedance));
            REQUIRE(impedance->setSetPoint(1, 1, 2, 3, 4, 5));
            double p = 0, v = 0, t = 0, k = 0, d = 0;
            REQUIRE(impedance->getSetPoint(1, p, v, t, k, d));
            REQUIRE(p == 1);
            REQUIRE(v == 2);
            REQUIRE(t == 3);
            REQUIRE(k == 4);
            REQUIRE(d == 5);
            yarp::dev::IPreciselyTimed* stamp = nullptr;
            REQUIRE(driver.view(stamp));
        } else
        {
            {
                yarp::dev::IThreeAxisGyroscopes* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::IThreeAxisLinearAccelerometers* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::IThreeAxisAngularAccelerometers* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::IThreeAxisMagnetometers* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::IPositionSensors* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::ILinearVelocitySensors* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::IOrientationSensors* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::ITemperatureSensors* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::ISixAxisForceTorqueSensors* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::IContactLoadCellArrays* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::IEncoderArrays* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
            {
                yarp::dev::ISkinPatches* sensor = nullptr;
                REQUIRE(driver.view(sensor));
            }
        }
        REQUIRE(driver.close());
    }
}

TEST_CASE("A native dinrail Device can request a YARP interface through the interop registry")
{
    dinrail::Parameters config;
    config.put("device", "dr_battery_fake");
    dinrail::Device device;
    REQUIRE(device.open(config));
    dinrail::IBatterySimulation* simulation = nullptr;
    REQUIRE(device.view(simulation));
    REQUIRE(simulation->setBatteryVoltage(42).ok());
    yarp::dev::IBattery* adapted = nullptr;
    REQUIRE(device.view(adapted));
    yarp::dev::IBattery* cached = nullptr;
    REQUIRE(device.view(cached));
    REQUIRE(cached == adapted);
    double value = 0;
    REQUIRE(adapted->getBatteryVoltage(value));
    REQUIRE(value == 42);
    REQUIRE(device.close());
}

namespace
{
class Joypad final : public dinrail::IDevice, public dinrail::IJoypadControl
{
public:
    bool open(const dinrail::Parameters&) override
    {
        return true;
    }
    bool close() override
    {
        return true;
    }
    bool getAxisCount(unsigned int& count) override
    {
        count = 2;
        return true;
    }
    bool getButtonCount(unsigned int& count) override
    {
        count = 3;
        return true;
    }
    bool getHatCount(unsigned int& count) override
    {
        count = 1;
        return true;
    }
    bool getAxis(unsigned int index, double& value) override
    {
        value = 0.75;
        return index < 2;
    }
    bool getButton(unsigned int index, float& value) override
    {
        value = 0.5f;
        return index < 3;
    }
    bool getHat(unsigned int index, unsigned char& value) override
    {
        value = 4;
        return index < 1;
    }
    bool reconnect() override
    {
        return true;
    }
    bool getLastEvent(dinrail::JoypadDeviceEvent& event) override
    {
        event = dinrail::JoypadDeviceEvent::NoEvent;
        return true;
    }
};
} // namespace

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
