// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpInteropPlugin.h>

#include <dinrail/YarpDeviceWrapper.h>
#include <dinrail/YarpPropertyConverter.h>

#include <sharedlibpp/SharedLibraryClassApi.h>

#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/os/Value.h>
#include <yarp/os/YarpPluginSelector.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace dinrail
{

namespace
{

// A directory that may contain YARP plugin libraries, with the expected suffix.
struct YarpLibraryLocation
{
    std::filesystem::path directory;
    std::string extension;
};

// Collect candidate library directories from the [search] sections reported by
// the selector (each relative_path is relative to the .ini that declares it).
std::vector<YarpLibraryLocation> collectYarpLibraryLocations(const yarp::os::Bottle& searchPaths)
{
    std::vector<YarpLibraryLocation> locations;

    for (std::size_t i = 0; i < searchPaths.size(); ++i)
    {
        yarp::os::Bottle* entry = searchPaths.get(i).asList();
        if (entry == nullptr)
        {
            continue;
        }

        const std::string extension = entry->check("extension", yarp::os::Value(".so")).asString();

        const std::string absolute = entry->check("path", yarp::os::Value("")).asString();
        if (!absolute.empty())
        {
            locations.push_back({std::filesystem::path(absolute), extension});
            continue;
        }

        const std::string inifile = entry->check("inifile", yarp::os::Value("")).asString();
        const std::string relative = entry->check("relative_path", yarp::os::Value("")).asString();
        if (inifile.empty() || relative.empty())
        {
            continue;
        }

        const std::filesystem::path directory
            = (std::filesystem::path(inifile).parent_path() / relative).lexically_normal();
        locations.push_back({directory, extension});
    }

    return locations;
}

// Resolve a YARP plugin library base name to a full path (mirrors how yarpdev
// reports device locations); falls back to the base name if it cannot be found.
std::string resolveYarpLibraryPath(const std::vector<YarpLibraryLocation>& locations,
                                   const std::string& library)
{
    if (library.empty())
    {
        return {};
    }

    for (const auto& location : locations)
    {
        for (const std::string& fileName :
             {library + location.extension, "lib" + library + location.extension})
        {
            const std::filesystem::path candidate = location.directory / fileName;
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec))
            {
                return candidate.string();
            }
        }
    }

    return library;
}

} // namespace

std::unique_ptr<dinrail::IDevice> YarpInteropPlugin::createDevice(const Parameters& config)
{
    yarp::os::Property yarpConfig = YarpPropertyConverter::toYarpProperty(config);

    auto yarpDriver = std::make_unique<yarp::dev::PolyDriver>();
    if (!yarpDriver->open(yarpConfig))
    {
        return nullptr;
    }

    return std::make_unique<YarpDeviceWrapper>(std::move(yarpDriver));
}

std::vector<DeviceInfo> YarpInteropPlugin::listDevices() const
{
    std::vector<DeviceInfo> devices;

    yarp::os::YarpPluginSelector selector;
    selector.scan();

    const std::vector<YarpLibraryLocation> locations
        = collectYarpLibraryLocations(selector.getSearchPath());

    const yarp::os::Bottle plugins = selector.getSelectedPlugins();
    for (std::size_t i = 0; i < plugins.size(); ++i)
    {
        const yarp::os::Value& options = plugins.get(i);
        if (options.check("type", yarp::os::Value("")).asString() != "device")
        {
            continue;
        }

        const std::string name = options.check("name", yarp::os::Value("")).asString();
        if (name.empty())
        {
            continue;
        }

        const std::string library = options.check("library", yarp::os::Value("")).asString();
        devices.push_back({name, resolveYarpLibraryPath(locations, library)});
    }

    return devices;
}

} // namespace dinrail

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_interop_yarp,
                               dinrail::YarpInteropPlugin,
                               dinrail::IInteropPlugin)
