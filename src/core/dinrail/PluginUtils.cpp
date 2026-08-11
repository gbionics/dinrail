// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/PluginUtils.h>

#include <dinrail/IInteropPlugin.h>

#include <sharedlibpp/SharedLibraryClassFactory.h>

#include <cstdlib>
#include <map>
#include <set>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace dinrail
{

namespace
{

// Extract plugin name from a shared library file name following the
// `<pluginPrefix>-<name>` convention, or an empty string if it does not match.
std::string
extractPluginNameFromLibraryFileName(const std::string& fileName, const std::string& pluginPrefix)
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
    const std::string prefix = "lib" + pluginPrefix + "-";
#if defined(__APPLE__)
    const std::string suffix = ".dylib";
#else
    const std::string suffix = ".so";
#endif

    if (fileName.rfind(prefix, 0) != 0)
    {
        return {};
    }

    const std::size_t suffixPos = fileName.find(suffix, prefix.size());
    if (suffixPos == std::string::npos || suffixPos == prefix.size())
    {
        return {};
    }

    return fileName.substr(prefix.size(), suffixPos - prefix.size());
#endif
}

template <class T>
std::unique_ptr<T, std::function<void(T*)>>
makeFactoryUnique(const sharedlibpp::SharedLibraryClassFactory<T>& factory)
{
    auto* factoryPtr = &factory;
    return std::unique_ptr<T, std::function<void(T*)>>(factory.create(), [factoryPtr](T* p) {
        if (p != nullptr && factoryPtr != nullptr)
        {
            factoryPtr->destroy(p);
        }
    });
}

} // namespace

std::string getSharedlibppLibraryNameFromDeviceName(const std::string& deviceName)
{
    return "dinrail-device-" + deviceName;
}

std::string getSharedlibppFactoryNameFromDeviceName(const std::string& deviceName)
{
    std::string factoryName = "dinrail_device_" + deviceName;
    for (char& c : factoryName)
    {
        if (c == '-')
        {
            c = '_';
        }
    }
    return factoryName;
}

std::string getSharedlibppLibraryNameFromInteropName(const std::string& interopName)
{
    return "dinrail-interop-" + interopName;
}

std::string getSharedlibppFactoryNameFromInteropName(const std::string& interopName)
{
    std::string factoryName = "dinrail_interop_" + interopName;
    for (char& c : factoryName)
    {
        if (c == '-')
        {
            c = '_';
        }
    }
    return factoryName;
}

std::vector<std::string> getAvailableInteropPluginNames()
{
    const auto plugins = getAvailableInteropPlugins();

    std::set<std::string> names;
    for (const auto& plugin : plugins)
    {
        names.insert(plugin.name);
    }

    return {names.begin(), names.end()};
}

std::vector<DeviceInfo> getAvailableNativeDevices()
{
    std::map<std::string, std::string> devicesByName;

    for (const auto& dir : getPluginSearchPaths())
    {
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec))
        {
            continue;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
        {
            std::error_code entryEc;
            if (!entry.is_regular_file(entryEc))
            {
                continue;
            }

            const std::string name
                = extractPluginNameFromLibraryFileName(entry.path().filename().string(),
                                                       "dinrail-device");
            if (!name.empty())
            {
                devicesByName.emplace(name, entry.path().string());
            }
        }
    }

    std::vector<DeviceInfo> devices;
    devices.reserve(devicesByName.size());
    for (const auto& [name, location] : devicesByName)
    {
        devices.push_back({name, location});
    }

    return devices;
}

std::vector<InteropPluginInfo> getAvailableInteropPlugins()
{
    std::map<std::string, std::string> pluginsByName;

    for (const auto& dir : getPluginSearchPaths())
    {
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec))
        {
            continue;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
        {
            std::error_code entryEc;
            if (!entry.is_regular_file(entryEc))
            {
                continue;
            }

            const std::string name
                = extractPluginNameFromLibraryFileName(entry.path().filename().string(),
                                                       "dinrail-interop");
            if (!name.empty())
            {
                pluginsByName.emplace(name, entry.path().string());
            }
        }
    }

    std::vector<InteropPluginInfo> plugins;
    plugins.reserve(pluginsByName.size());
    for (const auto& [name, location] : pluginsByName)
    {
        plugins.push_back({name, location});
    }

    return plugins;
}

std::vector<InteropDevices> getAvailableInteropDevices()
{
    std::vector<InteropDevices> result;

    for (const auto& interopPluginInfo : getAvailableInteropPlugins())
    {
        const std::string libraryName
            = getSharedlibppLibraryNameFromInteropName(interopPluginInfo.name);
        const std::string factoryName
            = getSharedlibppFactoryNameFromInteropName(interopPluginInfo.name);

        sharedlibpp::SharedLibraryClassFactory<dinrail::IInteropPlugin>
            factory(SHLIBPP_DEFAULT_START_CHECK,
                    SHLIBPP_DEFAULT_END_CHECK,
                    SHLIBPP_DEFAULT_SYSTEM_VERSION,
                    factoryName.c_str());

        for (const auto& path : getPluginSearchPaths())
        {
            factory.extendSearchPath(path.string());
        }

        bool ok = factory.open(libraryName.c_str(), factoryName.c_str());
        ok = ok && factory.isValid();
        if (!ok)
        {
            continue;
        }

        auto interop = makeFactoryUnique(factory);
        if (!interop)
        {
            continue;
        }

        std::vector<DeviceInfo> devices;
        const auto pluginDevices = interop->listDevices();
        devices.reserve(pluginDevices.size());
        for (const auto& device : pluginDevices)
        {
            devices.push_back({device.name, device.location});
        }

        result.push_back({interopPluginInfo, std::move(devices)});
    }

    return result;
}

AvailableDevices getAvailableDevices()
{
    return {getAvailableNativeDevices(), getAvailableInteropDevices()};
}

std::optional<std::string> getPathOfDinrailSharedLibrary()
{
    std::filesystem::path libraryLocation;
#ifndef _WIN32
    Dl_info addressInfo;
    int resVal = dladdr(reinterpret_cast<void*>(getPathOfDinrailSharedLibrary), &addressInfo);
    if (resVal == 0 || !addressInfo.dli_fname)
    {
        return {};
    } else
    {
        libraryLocation = addressInfo.dli_fname;
    }
#else
    char modulePath[MAX_PATH];
    HMODULE hm = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                              | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR)&getPathOfDinrailSharedLibrary,
                          &hm)
        == 0)
    {
        return {};
    }
    if (GetModuleFileNameA(hm, modulePath, sizeof(modulePath)) == 0)
    {
        return {};
    }

    libraryLocation = std::string(modulePath);
#endif

    return libraryLocation.parent_path().string();
}

std::vector<std::filesystem::path> getPluginSearchPaths()
{
    std::vector<std::filesystem::path> searchPaths;

    const std::optional<std::string> libPath = getPathOfDinrailSharedLibrary();
    if (libPath.has_value())
    {
        std::filesystem::path p(libPath.value());
        if (std::filesystem::exists(p))
        {
            searchPaths.emplace_back(libPath.value());
        }
    }

    if (const char* extra = std::getenv("DINRAIL_PLUGIN_PATH"))
    {
        std::stringstream ss(extra);
        std::string item;
#if defined(_WIN32)
        constexpr char pathSeparator = ';';
#else
        constexpr char pathSeparator = ':';
#endif
        while (std::getline(ss, item, pathSeparator))
        {
            if (!item.empty())
            {
                searchPaths.emplace_back(item);
            }
        }
    }

    return searchPaths;
}

} // namespace dinrail
