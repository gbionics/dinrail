// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IJointFault.h>
#include <dinrail/IMotor.h>
#include <dinrail/IMotorEncoders.h>
#include <dinrail/MultipleAnalogSensorsInterfaces.h>
#include <dinrail/Parameters.h>

#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/IEncodersTimed.h>

#include <vector>

namespace
{

struct ITestOnlyMissingInterface
{
    virtual ~ITestOnlyMissingInterface() = default;
    virtual void markerMethod() = 0;
};

} // namespace

TEST_CASE("View native YARP interfaces on a device opened via dinrail", "[yarp][compat]")
{
    // Open the YARP fakeMotionControl device through the dinrail YARP interop plugin.
    dinrail::Parameters opts;
    opts.put("device", "fakeMotionControl");

    dinrail::Parameters& general = opts.addGroup("GENERAL");
    general.put("Joints", 3);

    dinrail::Device device;
    REQUIRE(device.open(opts));

    // Check that yarp::dev::IAxisInfo is viewable and working
    yarp::dev::IAxisInfo* nativeAxisInfo = nullptr;
    REQUIRE(device.view(nativeAxisInfo));
    REQUIRE(nativeAxisInfo != nullptr);

    int nativeAxes = 0;
    REQUIRE(nativeAxisInfo->getAxes(&nativeAxes));
    REQUIRE(nativeAxes == 3);

    // Matching dinrail interfaces are provided by adapters owned by Device.
    dinrail::IAxisInfo* axisInfo = nullptr;
    REQUIRE(device.view(axisInfo));
    REQUIRE(axisInfo != nullptr);

    int axes = 0;
    REQUIRE(axisInfo->getAxes(&axes));
    REQUIRE(axes == nativeAxes);

    std::string nativeName;
    std::string name;
    REQUIRE(nativeAxisInfo->getAxisName(0, nativeName));
    REQUIRE(axisInfo->getAxisName(0, name));
    REQUIRE(name == nativeName);

    yarp::dev::JointTypeEnum nativeJointType = yarp::dev::VOCAB_JOINTTYPE_UNKNOWN;
    dinrail::JointType jointType = dinrail::JointType::UNKNOWN;
    REQUIRE(nativeAxisInfo->getJointType(0, nativeJointType));
    REQUIRE(axisInfo->getJointType(0, jointType));
    REQUIRE(jointType == dinrail::JointType::REVOLUTE);

    dinrail::IAxisInfo* cachedAxisInfo = nullptr;
    REQUIRE(device.view(cachedAxisInfo));
    REQUIRE(cachedAxisInfo == axisInfo);

    // Check that yarp::dev::IEncoders is also working
    yarp::dev::IEncoders* encoders = nullptr;
    REQUIRE(device.view(encoders));
    REQUIRE(encoders != nullptr);

    int encAxes = 0;
    REQUIRE(encoders->getAxes(&encAxes));
    REQUIRE(encAxes == 3);

    yarp::dev::IEncodersTimed* nativeTimedEncoders = nullptr;
    REQUIRE(device.view(nativeTimedEncoders));
    REQUIRE(nativeTimedEncoders != nullptr);
    double nativePositions[3]{};
    double nativeTimestamps[3]{};
    REQUIRE(nativeTimedEncoders->getEncodersTimed(nativePositions, nativeTimestamps));

    dinrail::IEncoders* adaptedEncoders = nullptr;
    REQUIRE(device.view(adaptedEncoders));
    REQUIRE(adaptedEncoders != nullptr);
    REQUIRE(adaptedEncoders->getAxes(&encAxes));
    REQUIRE(encAxes == 3);

    std::vector<double> positions;
    std::vector<double> timestamps;
    REQUIRE(adaptedEncoders->getEncodersTimed(positions, timestamps));
    REQUIRE(positions.size() == 3);
    REQUIRE(timestamps.size() == 3);

    dinrail::IMotor* motor = nullptr;
    dinrail::IMotorEncoders* motorEncoders = nullptr;
    dinrail::IJointFault* jointFault = nullptr;
    REQUIRE(device.view(motor));
    REQUIRE(device.view(motorEncoders));
    REQUIRE(device.view(jointFault));

    int motors = 0;
    REQUIRE(motor->getNumberOfMotors(&motors));
    REQUIRE(motors == 3);
    REQUIRE(motorEncoders->getNumberOfMotorEncoders(&motors));
    REQUIRE(motors == 3);

    // The wrapped yarp::dev::DeviceDriver must be viewable.
    yarp::dev::DeviceDriver* nativeDriver = nullptr;
    REQUIRE(device.view(nativeDriver));
    REQUIRE(nativeDriver != nullptr);

    // A test-local interface that the wrapped device does not implement must fail to resolve.
    ITestOnlyMissingInterface* missing = nullptr;
    REQUIRE_FALSE(device.view(missing));
    REQUIRE(missing == nullptr);

    REQUIRE(device.close());
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
