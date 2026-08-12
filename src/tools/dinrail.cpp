// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <CLI/CLI.hpp>

#include <dinrail/PluginUtils.h>
#include <dinrail/RuntimeContext.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace
{
std::vector<std::filesystem::path> getCandidatePluginDirs()
{
    return dinrail::getPluginSearchPaths();
}

} // namespace

int main(int argc, char** argv)
{
    CLI::App app{"dinrail command line tool"};

    bool listDevices = false;
    bool showSearchPath = false;
    auto* devSubcommand = app.add_subcommand("dev", "Developer-oriented commands");
    devSubcommand->add_flag("--list", listDevices, "List available dinrail device plugins");
    devSubcommand->add_flag("--show-search-path",
                            showSearchPath,
                            "List directories considered when searching dinrail plugins");

    bool listInterop = false;
    bool interopShowSearchPath = false;
    auto* interopSubcommand = app.add_subcommand("interop", "Interop plugin commands (debugging)");
    interopSubcommand->add_flag("--list", listInterop, "List available interop plugins");
    interopSubcommand->add_flag("--show-search-path",
                                interopShowSearchPath,
                                "List directories considered when searching dinrail plugins");

    CLI11_PARSE(app, argc, argv);

    if (interopSubcommand->parsed() != 0)
    {
        const auto searchPaths = getCandidatePluginDirs();
        bool actionRequested = false;

        if (interopShowSearchPath)
        {
            actionRequested = true;
            if (searchPaths.empty())
            {
                std::cout << "No search paths found.\n";
            } else
            {
                for (const auto& path : searchPaths)
                {
                    std::cout << path.lexically_normal().string() << "\n";
                }
            }
        }

        if (listInterop)
        {
            actionRequested = true;
            const auto interopPlugins = dinrail::getAvailableInteropPlugins();
            if (interopPlugins.empty())
            {
                std::cout << "No interop plugins found.\n";
                return 0;
            }

            for (const auto& plugin : interopPlugins)
            {
                std::cout << plugin.name << "\t" << plugin.location << "\n";
            }
            return 0;
        }

        if (actionRequested)
        {
            return 0;
        }

        std::cout << interopSubcommand->help() << std::endl;
        return 1;
    }

    if (devSubcommand->parsed() == 0)
    {
        std::cout << app.help() << std::endl;
        return 0;
    }

    const auto searchPaths = getCandidatePluginDirs();
    bool actionRequested = false;

    if (showSearchPath)
    {
        actionRequested = true;
        if (searchPaths.empty())
        {
            std::cout << "No search paths found.\n";
        } else
        {
            for (const auto& path : searchPaths)
            {
                std::cout << path.lexically_normal().string() << "\n";
            }
        }
    }

    if (listDevices)
    {
        actionRequested = true;

        const auto available = dinrail::getAvailableDevices();

        bool anyInteropDevice = false;
        for (const auto& group : available.interopDevices)
        {
            if (!group.devices.empty())
            {
                anyInteropDevice = true;
                break;
            }
        }

        if (available.nativeDevices.empty() && !anyInteropDevice)
        {
            std::cout << "No devices found.\n";
            return 0;
        }

        std::cout << "dinrail devices:\n";
        if (available.nativeDevices.empty())
        {
            std::cout << "  (none)\n";
        } else
        {
            for (const auto& device : available.nativeDevices)
            {
                std::cout << "  " << device.name << "\t" << device.location << "\n";
            }
        }

        for (const auto& group : available.interopDevices)
        {
            if (group.devices.empty())
            {
                continue;
            }

            std::cout << "\n" << group.interopPlugin.name << " devices (via interop):\n";
            for (const auto& device : group.devices)
            {
                std::cout << "  " << device.name << "\t" << device.location << "\n";
            }
        }

        return 0;
    }

    if (actionRequested)
    {
        return 0;
    }

    std::cout << devSubcommand->help() << std::endl;
    return 1;
}
