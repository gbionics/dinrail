// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/Parameters.h>
#include <dinrail/RuntimeContext.h>

#include "interop/IFooTest.h"

#include <algorithm>
#include <string>

TEST_CASE("Available interop plugins are discovered on the search path", "[interop]")
{
    const auto plugins = dinrail::RuntimeContext::getDefault().listInteropPlugins();

    const auto hasPlugin = [&](const std::string& name) {
        return std::any_of(plugins.begin(), plugins.end(), [&](const auto& plugin) {
            return plugin.name == name;
        });
    };

    REQUIRE(hasPlugin("testalpha"));
    REQUIRE(hasPlugin("testbeta"));
}

TEST_CASE("view() resolves a custom interface added by an interop plugin", "[interop]")
{
    dinrail::Parameters opts;
    opts.put("device", std::string("alpha_device"));

    dinrail::Device device;
    REQUIRE(device.open(opts));

    // IFooTest is unknown to dinrail; it is resolved via the interop device's
    // IInterfaceView implementation.
    dinrail::test::IFooTest* foo = nullptr;
    REQUIRE(device.view(foo));
    REQUIRE(foo != nullptr);
    REQUIRE(foo->tag() == "alpha");

    REQUIRE(device.close());
}

TEST_CASE("Interop plugins are tried until one handles the device", "[interop]")
{
    // Only the beta plugin handles "beta_device"; the alpha plugin is skipped.
    dinrail::Parameters opts;
    opts.put("device", std::string("beta_device"));

    dinrail::Device device;
    REQUIRE(device.open(opts));

    dinrail::test::IFooTest* foo = nullptr;
    REQUIRE(device.view(foo));
    REQUIRE(foo != nullptr);
    REQUIRE(foo->tag() == "beta");

    REQUIRE(device.close());
}

TEST_CASE("view() caches interface translations supplied by interop plugins", "[interop]")
{
    dinrail::Parameters opts;
    opts.put("device", std::string("alpha_device"));

    dinrail::Device device;
    REQUIRE(device.open(opts));

    dinrail::test::ITranslatedFooTest* translated = nullptr;
    REQUIRE(device.view(translated));
    REQUIRE(translated != nullptr);
    REQUIRE(translated->translatedTag() == "translated:alpha");

    dinrail::test::ITranslatedFooTest* cached = nullptr;
    REQUIRE(device.view(cached));
    REQUIRE(cached == translated);

    REQUIRE(device.close());
}

TEST_CASE("Interop plugins report the devices they can open", "[interop]")
{
    const auto groups = dinrail::RuntimeContext::getDefault().listInteropDevices();

    const auto deviceIsListedBy
        = [&](const std::string& interopPlugin, const std::string& deviceName) {
              for (const auto& group : groups)
              {
                  if (group.interopPlugin.name != interopPlugin)
                  {
                      continue;
                  }
                  for (const auto& device : group.devices)
                  {
                      if (device.name == deviceName)
                      {
                          return true;
                      }
                  }
              }
              return false;
          };

    REQUIRE(deviceIsListedBy("testalpha", "alpha_device"));
    REQUIRE(deviceIsListedBy("testbeta", "beta_device"));
}
