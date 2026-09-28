// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "interop/IFooTest.h"

#include <catch2/catch_test_macros.hpp>
#include <dinrail/Device.h>

TEST_CASE("Interop creation and registration can query native child interfaces")
{
    // Use a separate executable so the default context has no registered adapters.
    // The plugin's child devices use that same context.
    dinrail::Parameters config;
    config.put("device", "reentrant_device");
    config.put("dinrail_device_type", "testreentrant");
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        dinrail::Device device;
        REQUIRE(device.open(config));
        dinrail::test::IFooTest* foo = nullptr;
        REQUIRE(device.view(foo));
        REQUIRE(foo->tag() == "reentrant");
        REQUIRE(device.close());
    }
}
