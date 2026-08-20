// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
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
