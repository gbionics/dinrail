// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/Parameters.h>

#include <yarp/dev/ControlBoardInterfaces.h>

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

    // The YARP interop plugin translates the native interface to the matching
    // dinrail interface when the latter is requested.
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
    if (nativeJointType == yarp::dev::VOCAB_JOINTTYPE_REVOLUTE)
    {
        REQUIRE(jointType == dinrail::JointType::REVOLUTE);
    } else if (nativeJointType == yarp::dev::VOCAB_JOINTTYPE_PRISMATIC)
    {
        REQUIRE(jointType == dinrail::JointType::PRISMATIC);
    } else
    {
        REQUIRE(jointType == dinrail::JointType::UNKNOWN);
    }

    // Successful translations are cached, giving callers a stable pointer.
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
