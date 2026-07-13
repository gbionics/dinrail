// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <CLI/CLI.hpp>

#include <dinrail/PluginUtils.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace
{

std::string extractPluginNameFromLibrary(const std::string& fileName,
                                         const std::string& pluginPrefix)
{
#if defined(_WIN32)
    const std::string prefix = pluginPrefix + "-";
    const std::string suffix = ".dll";

    if (fileName.size() > prefix.size() + suffix.size() && fileName.rfind(prefix, 0) == 0
        && fileName.substr(fileName.size() - suffix.size()) == suffix)
    {
        return fileName.substr(prefix.size(), fileName.size() - prefix.size() - suffix.size());
    }
    return {};
#else
#if defined(__APPLE__)
    const std::string suffix = ".dylib";
#else
    const std::string suffix = ".so";
#endif
    const std::string prefix = "lib" + pluginPrefix + "-";

    if (fileName.rfind(prefix, 0) != 0)
    {
        return {};
    }

    const std::size_t suffixPos = fileName.find(suffix, prefix.size());
    if (suffixPos == std::string::npos)
    {
        return {};
    }

    if (suffixPos == prefix.size())
    {
        return {};
    }

    return fileName.substr(prefix.size(), suffixPos - prefix.size());
#endif
}

std::vector<std::filesystem::path> getCandidatePluginDirs()
{
    return dinrail::getPluginSearchPaths();
}

std::vector<std::string> findAvailableDevices()
{
    std::set<std::string> devices;
    static const std::string devicePluginPrefix = "dinrail-device";

    for (const auto& dir : getCandidatePluginDirs())
    {
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec))
        {
            continue;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
        {
            if (ec || !entry.is_regular_file(ec))
            {
                continue;
            }

            const auto deviceName = extractPluginNameFromLibrary(
                entry.path().filename().string(), devicePluginPrefix);
            if (!deviceName.empty())
            {
                devices.insert(deviceName);
            }
        }
    }

    return {devices.begin(), devices.end()};
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

    CLI11_PARSE(app, argc, argv);

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
        }
        else
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
        const auto devices = findAvailableDevices();
        if (devices.empty())
        {
            std::cout << "No devices found.\n";
            return 0;
        }

        for (const auto& d : devices)
        {
            std::cout << d << "\n";
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
