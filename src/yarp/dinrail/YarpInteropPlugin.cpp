// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpInteropPlugin.h>

#include <dinrail/DinrailDeviceFromYarp.h>
#include <dinrail/YarpBatteryAdapters.h>
#include <dinrail/YarpJoypadAdapters.h>
#include <dinrail/YarpMultipleAnalogSensorsAdapters.h>
#include <dinrail/YarpPreciselyTimedAdapters.h>
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

    return std::make_unique<DinrailDeviceFromYarp>(std::move(yarpDriver));
}

void YarpInteropPlugin::registerInterfaceAdapters(InterfaceAdapterRegistry& registry)
{
    registry.add<yarp::dev::IThreeAxisGyroscopes,
                 dinrail::IThreeAxisGyroscopes,
                 InterfaceAdapter<yarp::dev::IThreeAxisGyroscopes, dinrail::IThreeAxisGyroscopes>>();
    registry.add<dinrail::IThreeAxisGyroscopes,
                 yarp::dev::IThreeAxisGyroscopes,
                 InterfaceAdapter<dinrail::IThreeAxisGyroscopes, yarp::dev::IThreeAxisGyroscopes>>();
    registry.add<yarp::dev::IThreeAxisLinearAccelerometers,
                 dinrail::IThreeAxisLinearAccelerometers,
                 InterfaceAdapter<yarp::dev::IThreeAxisLinearAccelerometers,
                                  dinrail::IThreeAxisLinearAccelerometers>>();
    registry.add<dinrail::IThreeAxisLinearAccelerometers,
                 yarp::dev::IThreeAxisLinearAccelerometers,
                 InterfaceAdapter<dinrail::IThreeAxisLinearAccelerometers,
                                  yarp::dev::IThreeAxisLinearAccelerometers>>();
    registry.add<yarp::dev::IThreeAxisAngularAccelerometers,
                 dinrail::IThreeAxisAngularAccelerometers,
                 InterfaceAdapter<yarp::dev::IThreeAxisAngularAccelerometers,
                                  dinrail::IThreeAxisAngularAccelerometers>>();
    registry.add<dinrail::IThreeAxisAngularAccelerometers,
                 yarp::dev::IThreeAxisAngularAccelerometers,
                 InterfaceAdapter<dinrail::IThreeAxisAngularAccelerometers,
                                  yarp::dev::IThreeAxisAngularAccelerometers>>();
    registry.add<
        yarp::dev::IThreeAxisMagnetometers,
        dinrail::IThreeAxisMagnetometers,
        InterfaceAdapter<yarp::dev::IThreeAxisMagnetometers, dinrail::IThreeAxisMagnetometers>>();
    registry.add<
        dinrail::IThreeAxisMagnetometers,
        yarp::dev::IThreeAxisMagnetometers,
        InterfaceAdapter<dinrail::IThreeAxisMagnetometers, yarp::dev::IThreeAxisMagnetometers>>();
    registry.add<yarp::dev::IPositionSensors,
                 dinrail::IPositionSensors,
                 InterfaceAdapter<yarp::dev::IPositionSensors, dinrail::IPositionSensors>>();
    registry.add<dinrail::IPositionSensors,
                 yarp::dev::IPositionSensors,
                 InterfaceAdapter<dinrail::IPositionSensors, yarp::dev::IPositionSensors>>();
    registry
        .add<yarp::dev::ILinearVelocitySensors,
             dinrail::ILinearVelocitySensors,
             InterfaceAdapter<yarp::dev::ILinearVelocitySensors, dinrail::ILinearVelocitySensors>>();
    registry
        .add<dinrail::ILinearVelocitySensors,
             yarp::dev::ILinearVelocitySensors,
             InterfaceAdapter<dinrail::ILinearVelocitySensors, yarp::dev::ILinearVelocitySensors>>();
    registry.add<yarp::dev::IOrientationSensors,
                 dinrail::IOrientationSensors,
                 InterfaceAdapter<yarp::dev::IOrientationSensors, dinrail::IOrientationSensors>>();
    registry.add<dinrail::IOrientationSensors,
                 yarp::dev::IOrientationSensors,
                 InterfaceAdapter<dinrail::IOrientationSensors, yarp::dev::IOrientationSensors>>();
    registry.add<yarp::dev::ITemperatureSensors,
                 dinrail::ITemperatureSensors,
                 InterfaceAdapter<yarp::dev::ITemperatureSensors, dinrail::ITemperatureSensors>>();
    registry.add<dinrail::ITemperatureSensors,
                 yarp::dev::ITemperatureSensors,
                 InterfaceAdapter<dinrail::ITemperatureSensors, yarp::dev::ITemperatureSensors>>();
    registry.add<yarp::dev::ISixAxisForceTorqueSensors,
                 dinrail::ISixAxisForceTorqueSensors,
                 InterfaceAdapter<yarp::dev::ISixAxisForceTorqueSensors,
                                  dinrail::ISixAxisForceTorqueSensors>>();
    registry.add<dinrail::ISixAxisForceTorqueSensors,
                 yarp::dev::ISixAxisForceTorqueSensors,
                 InterfaceAdapter<dinrail::ISixAxisForceTorqueSensors,
                                  yarp::dev::ISixAxisForceTorqueSensors>>();
    registry
        .add<yarp::dev::IContactLoadCellArrays,
             dinrail::IContactLoadCellArrays,
             InterfaceAdapter<yarp::dev::IContactLoadCellArrays, dinrail::IContactLoadCellArrays>>();
    registry
        .add<dinrail::IContactLoadCellArrays,
             yarp::dev::IContactLoadCellArrays,
             InterfaceAdapter<dinrail::IContactLoadCellArrays, yarp::dev::IContactLoadCellArrays>>();
    registry.add<yarp::dev::IEncoderArrays,
                 dinrail::IEncoderArrays,
                 InterfaceAdapter<yarp::dev::IEncoderArrays, dinrail::IEncoderArrays>>();
    registry.add<dinrail::IEncoderArrays,
                 yarp::dev::IEncoderArrays,
                 InterfaceAdapter<dinrail::IEncoderArrays, yarp::dev::IEncoderArrays>>();
    registry.add<yarp::dev::ISkinPatches,
                 dinrail::ISkinPatches,
                 InterfaceAdapter<yarp::dev::ISkinPatches, dinrail::ISkinPatches>>();
    registry.add<dinrail::ISkinPatches,
                 yarp::dev::ISkinPatches,
                 InterfaceAdapter<dinrail::ISkinPatches, yarp::dev::ISkinPatches>>();
    registry.add<dinrail::IJoypadControl,
                 yarp::dev::IJoypadController,
                 InterfaceAdapter<dinrail::IJoypadControl, yarp::dev::IJoypadController>>();
    registry.add<yarp::dev::IJoypadController,
                 dinrail::IJoypadControl,
                 InterfaceAdapter<yarp::dev::IJoypadController, dinrail::IJoypadControl>>();
    registry.add<dinrail::IPreciselyTimed,
                 yarp::dev::IPreciselyTimed,
                 InterfaceAdapter<dinrail::IPreciselyTimed, yarp::dev::IPreciselyTimed>>();
    registry.add<yarp::dev::IPreciselyTimed,
                 dinrail::IPreciselyTimed,
                 InterfaceAdapter<yarp::dev::IPreciselyTimed, dinrail::IPreciselyTimed>>();
    registry.add<dinrail::IBattery,
                 yarp::dev::IBattery,
                 InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>>();
    registry.add<yarp::dev::IBattery,
                 dinrail::IBattery,
                 InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>>();
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
