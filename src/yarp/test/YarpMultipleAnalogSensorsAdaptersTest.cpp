// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <DrMultipleAnalogSensorsFakeYarp.h>
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

TEST_CASE("Adapt YARP multiple analog sensor interfaces", "[yarp][compat]")
{
    dinrail::Parameters opts;
    opts.put("device", "fakeIMU");

    dinrail::Device device;
    REQUIRE(device.open(opts));

    dinrail::IThreeAxisGyroscopes* gyroscopes = nullptr;
    dinrail::IThreeAxisLinearAccelerometers* accelerometers = nullptr;
    dinrail::IThreeAxisMagnetometers* magnetometers = nullptr;
    dinrail::IOrientationSensors* orientations = nullptr;
    REQUIRE(device.view(gyroscopes));
    REQUIRE(device.view(accelerometers));
    REQUIRE(device.view(magnetometers));
    REQUIRE(device.view(orientations));

    REQUIRE(gyroscopes->getNrOfThreeAxisGyroscopes() == 1);
    REQUIRE(gyroscopes->getThreeAxisGyroscopeStatus(0) == dinrail::MAS_OK);

    std::vector<double> measurement;
    double timestamp = 0.0;
    REQUIRE(gyroscopes->getThreeAxisGyroscopeMeasure(0, measurement, timestamp));
    REQUIRE(measurement.size() == 3);

    REQUIRE(device.close());
}

TEST_CASE("A native sensor device exposes a cached YARP gyroscope adapter")
{
    dinrail::Device device;
    dinrail::Parameters config;
    config.put("device", "dr_multiplenalogsensors_fake");
    config.put("dinrail_device_type", "dinrail");
    REQUIRE(device.open(config));
    dinrail::IThreeAxisGyroscopesSimulation* simulation = nullptr;
    REQUIRE(device.view(simulation));
    const std::vector<double> expected{1.5, 2.5, 3.5};
    REQUIRE(simulation->setThreeAxisGyroscopeMeasure(0, expected, 42.25));
    yarp::dev::IThreeAxisGyroscopes* adapted = nullptr;
    REQUIRE(device.view(adapted));
    yarp::dev::IThreeAxisGyroscopes* cached = nullptr;
    REQUIRE(device.view(cached));
    REQUIRE(cached == adapted);
    yarp::sig::Vector measurement;
    double timestamp = 0;
    REQUIRE(adapted->getThreeAxisGyroscopeMeasure(0, measurement, timestamp));
    REQUIRE(measurement.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        REQUIRE(measurement[i] == expected[i]);
    }
    REQUIRE(timestamp == 42.25);
    REQUIRE(device.close());
}

#ifdef DINRAIL_HAS_YARP_MAS_WRAPPER
namespace
{
template <class Interface> void requireInstalledInterface(yarp::dev::PolyDriver& driver)
{
    Interface* interface = nullptr;
    REQUIRE(driver.view(interface));
    REQUIRE(interface != nullptr);
}
} // namespace

TEST_CASE("The sensor fake loads as an installed YARP plugin with all sensor interfaces")
{
    yarp::os::Property config;
    config.put("device", "dr_multiplenalogsensors_fake");
    yarp::dev::PolyDriver driver;
    REQUIRE(driver.open(config));
    requireInstalledInterface<yarp::dev::IThreeAxisGyroscopes>(driver);
    requireInstalledInterface<yarp::dev::IThreeAxisLinearAccelerometers>(driver);
    requireInstalledInterface<yarp::dev::IThreeAxisAngularAccelerometers>(driver);
    requireInstalledInterface<yarp::dev::IThreeAxisMagnetometers>(driver);
    requireInstalledInterface<yarp::dev::IPositionSensors>(driver);
    requireInstalledInterface<yarp::dev::ILinearVelocitySensors>(driver);
    requireInstalledInterface<yarp::dev::IOrientationSensors>(driver);
    requireInstalledInterface<yarp::dev::ITemperatureSensors>(driver);
    requireInstalledInterface<yarp::dev::ISixAxisForceTorqueSensors>(driver);
    requireInstalledInterface<yarp::dev::IContactLoadCellArrays>(driver);
    requireInstalledInterface<yarp::dev::IEncoderArrays>(driver);
    requireInstalledInterface<yarp::dev::ISkinPatches>(driver);
    yarp::dev::IThreeAxisGyroscopes* gyroscopes = nullptr;
    REQUIRE(driver.view(gyroscopes));
    REQUIRE(gyroscopes->getNrOfThreeAxisGyroscopes() == 1);
    REQUIRE(gyroscopes->getThreeAxisGyroscopeStatus(0) == yarp::dev::MAS_OK);
    yarp::sig::Vector measurement;
    double timestamp = 0;
    REQUIRE(gyroscopes->getThreeAxisGyroscopeMeasure(0, measurement, timestamp));
    REQUIRE(measurement.size() == 3);
    REQUIRE(driver.close());
}
#endif
