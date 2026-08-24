// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/Parameters.h>
#include <dinrail/RuntimeContext.h>

#include <algorithm>
#include <string>

TEST_CASE("Native module plugins are discovered and loaded by their exact path", "[plugin]")
{
    const auto devices = dinrail::RuntimeContext::getDefault().listNativeDevices();
    const auto found = std::find_if(devices.begin(), devices.end(), [](const auto& device) {
        return device.name == "testnative";
    });

    REQUIRE(found != devices.end());
    REQUIRE_FALSE(found->location.empty());

    dinrail::Parameters opts;
    opts.put("device", std::string("testnative"));

    dinrail::Device device;
    REQUIRE(device.open(opts));
    REQUIRE(device.close());
}
