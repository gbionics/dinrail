// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Device.h>
#include <dinrail/Parameters.h>
#include <dinrail/RuntimeContext.h>

#include "interop/IFooTest.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <barrier>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace
{

class CerrCapture
{
public:
    CerrCapture()
        : m_previous(std::cerr.rdbuf(m_stream.rdbuf()))
    {
    }

    ~CerrCapture()
    {
        std::cerr.rdbuf(m_previous);
    }

    std::string str() const
    {
        return m_stream.str();
    }

private:
    std::ostringstream m_stream;
    std::streambuf* m_previous;
};

} // namespace

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
    REQUIRE(foo->tag().rfind("alpha-", 0) == 0);

    REQUIRE(device.close());
}

TEST_CASE("An interop plugin instance is reused by devices in the same runtime context",
          "[interop]")
{
    dinrail::RuntimeContext context;
    dinrail::Parameters opts;
    opts.put("device", std::string("alpha_device"));

    dinrail::Device first(context);
    dinrail::Device second(context);
    REQUIRE(first.open(opts));
    REQUIRE(second.open(opts));

    dinrail::test::IFooTest* firstFoo = nullptr;
    dinrail::test::IFooTest* secondFoo = nullptr;
    REQUIRE(first.view(firstFoo));
    REQUIRE(second.view(secondFoo));

    // The alpha test plugin stores this counter in the plugin object itself.
    // Seeing consecutive values proves both devices used the same instance.
    REQUIRE(firstFoo->tag() == "alpha-1");
    REQUIRE(secondFoo->tag() == "alpha-2");
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

TEST_CASE("dinrail_device_type selects a named interop plugin", "[interop]")
{
    dinrail::Parameters opts;
    opts.put("device", std::string("beta_device"));
    opts.put("dinrail_device_type", std::string("testbeta"));

    dinrail::Device device;
    REQUIRE(device.open(opts));

    dinrail::test::IFooTest* foo = nullptr;
    REQUIRE(device.view(foo));
    REQUIRE(foo != nullptr);
    REQUIRE(foo->tag() == "beta");

    REQUIRE(device.close());
}

TEST_CASE("dinrail_device_type restricts device plugin selection", "[interop]")
{
    dinrail::Parameters opts;
    opts.put("device", std::string("alpha_device"));

    SECTION("native selection does not fall back to interop plugins")
    {
        opts.put("dinrail_device_type", std::string("dinrail"));
        dinrail::Device device;
        REQUIRE_FALSE(device.open(opts));
    }

    SECTION("a different interop plugin is not tried")
    {
        opts.put("dinrail_device_type", std::string("testbeta"));
        dinrail::Device device;
        REQUIRE_FALSE(device.open(opts));
    }

    SECTION("an unavailable interop plugin fails")
    {
        opts.put("dinrail_device_type", std::string("missing"));
        dinrail::Device device;
        REQUIRE_FALSE(device.open(opts));
    }
}

TEST_CASE("A failed open reports the device and dinrail_device_type", "[interop]")
{
    const std::vector<std::string> deviceTypes{"auto", "dinrail", "testbeta", "missing"};

    for (const auto& deviceType : deviceTypes)
    {
        dinrail::Parameters opts;
        opts.put("device", std::string("unhandled_device"));
        opts.put("dinrail_device_type", deviceType);

        dinrail::Device device;
        CerrCapture captured;
        const bool opened = device.open(opts);

        REQUIRE_FALSE(opened);
        const std::string expected = "dinrail::Device: failed to open device 'unhandled_device' "
                                     "with dinrail_device_type '"
                                     + deviceType + "'";
        REQUIRE(captured.str().find(expected) != std::string::npos);
    }
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

TEST_CASE("view() caches interface adapters supplied by interop plugins", "[interop]")
{
    dinrail::Parameters opts;
    opts.put("device", std::string("alpha_device"));

    dinrail::RuntimeContext context;
    dinrail::Device device(context);
    REQUIRE(device.open(opts));

    dinrail::test::IAdaptedFooTest* adapted = nullptr;
    REQUIRE(device.view(adapted));
    REQUIRE(adapted != nullptr);
    REQUIRE(adapted->adaptedTag() == "adapted:alpha-1");

    dinrail::test::IAdaptedFooTest* cached = nullptr;
    REQUIRE(device.view(cached));
    REQUIRE(cached == adapted);

    REQUIRE(device.close());
}

TEST_CASE("Concurrent first queries retain the same live adapter", "[interop]")
{
    dinrail::RuntimeContext context;
    dinrail::Parameters opts;
    opts.put("device", "alpha_device");
    for (int round = 0; round < 8; ++round)
    {
        dinrail::Device device(context);
        REQUIRE(device.open(opts));
        std::array<dinrail::test::IAdaptedFooTest*, 16> interfaces{};
        std::barrier start(static_cast<std::ptrdiff_t>(interfaces.size()));
        std::atomic<int> successes{0};
        std::vector<std::thread> workers;
        for (std::size_t index = 0; index < interfaces.size(); ++index)
        {
            workers.emplace_back([&, index] {
                start.arrive_and_wait();
                if (device.view(interfaces[index]))
                {
                    ++successes;
                }
            });
        }
        for (auto& worker : workers)
        {
            worker.join();
        }
        REQUIRE(successes == interfaces.size());
        REQUIRE(interfaces.front() != nullptr);
        for (auto* interface : interfaces)
        {
            REQUIRE(interface == interfaces.front());
            REQUIRE(interface->adaptedTag() == "adapted:alpha-" + std::to_string(round + 1));
        }
    }
}

namespace
{
// Restore the process environment and remove the fixture even on a failed assertion.
class LatePluginSearchPath
{
public:
    LatePluginSearchPath()
    {
        if (const char* previous = std::getenv("DINRAIL_PLUGIN_PATH"))
        {
            m_previous = previous;
        }
        m_directory
            = std::filesystem::temp_directory_path()
              / ("dinrail-late-plugin-"
                 + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directory(m_directory);
        std::string path = m_previous.value_or("");
#ifdef _WIN32
        path += ";";
#else
        path += ":";
#endif
        path += m_directory.string();
        setPath(path.c_str());
    }

    ~LatePluginSearchPath()
    {
        setPath(m_previous ? m_previous->c_str() : nullptr);
        std::error_code error;
        std::filesystem::remove_all(m_directory, error);
    }

    std::filesystem::path destination() const
    {
        return m_directory / std::filesystem::path(DINRAIL_TEST_LATE_PLUGIN).filename();
    }

private:
    static void setPath(const char* value)
    {
#ifdef _WIN32
        _putenv_s("DINRAIL_PLUGIN_PATH", value ? value : "");
#else
        if (value)
        {
            setenv("DINRAIL_PLUGIN_PATH", value, 1);
        } else
        {
            unsetenv("DINRAIL_PLUGIN_PATH");
        }
#endif
    }

    std::optional<std::string> m_previous;
    std::filesystem::path m_directory;
};
} // namespace

TEST_CASE("A context registers adapters from newly available plugins", "[interop]")
{
    LatePluginSearchPath searchPath;
    dinrail::RuntimeContext context;
    dinrail::Device device(context);
    dinrail::Parameters opts;
    opts.put("device", "alpha_device");
    REQUIRE(device.open(opts));
    dinrail::test::ILateAdaptedFooTest* adapted = nullptr;
    REQUIRE_FALSE(device.view(adapted));

    // A discovered but unloadable library must also remain retryable.
    std::filesystem::copy_file(DINRAIL_TEST_LATE_PLUGIN, searchPath.destination());
    std::filesystem::resize_file(searchPath.destination(), 0);
    REQUIRE_FALSE(device.view(adapted));
    std::filesystem::copy_file(DINRAIL_TEST_LATE_PLUGIN,
                               searchPath.destination(),
                               std::filesystem::copy_options::overwrite_existing);
    REQUIRE(device.view(adapted));
    REQUIRE(adapted->adaptedTag() == "late:alpha-1");
    dinrail::test::ILateAdaptedFooTest* cached = nullptr;
    REQUIRE(device.view(cached));
    REQUIRE(cached == adapted);

    // A new device rescans the registry without invoking successful hooks again.
    dinrail::Device second(context);
    REQUIRE(second.open(opts));
    REQUIRE(second.view(adapted));
    REQUIRE(adapted->adaptedTag() == "late:alpha-2");
}
