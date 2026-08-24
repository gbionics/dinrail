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

struct PluginLibraryFileNameConvention
{
    std::string libraryStemPrefix;
    std::string fileNamePrefix;
    std::vector<std::string> suffixes;

    std::string libraryStem(const std::string& pluginName) const
    {
        return libraryStemPrefix + pluginName;
    }

    std::vector<std::string> fileNameCandidates(const std::string& pluginName) const
    {
        std::vector<std::string> candidates;
        candidates.reserve(suffixes.size());
        for (const auto& suffix : suffixes)
        {
            candidates.push_back(fileNamePrefix + pluginName + suffix);
        }
        return candidates;
    }

    // Extract the plugin name, or return an empty string if the file does not
    // follow this convention.
    std::string pluginNameFromFileName(const std::string& fileName) const
    {
        if (fileName.rfind(fileNamePrefix, 0) != 0)
        {
            return {};
        }

        for (const auto& suffix : suffixes)
        {
            if (fileName.size() > fileNamePrefix.size() + suffix.size()
                && fileName.compare(fileName.size() - suffix.size(), suffix.size(), suffix) == 0)
            {
                return fileName.substr(fileNamePrefix.size(),
                                       fileName.size() - fileNamePrefix.size() - suffix.size());
            }
        }

        return {};
    }
};

PluginLibraryFileNameConvention makePluginLibraryFileNameConvention(const std::string& pluginPrefix)
{
    const std::string libraryStemPrefix = pluginPrefix + "-";
#if defined(_WIN32)
    return {libraryStemPrefix, libraryStemPrefix, {".dll"}};
#elif defined(__APPLE__)
    return {libraryStemPrefix, "lib" + libraryStemPrefix, {".dylib", ".so"}};
#else
    return {libraryStemPrefix, "lib" + libraryStemPrefix, {".so"}};
#endif
}

const PluginLibraryFileNameConvention nativePluginConvention
    = makePluginLibraryFileNameConvention("dinrail-device");
const PluginLibraryFileNameConvention interopPluginConvention
    = makePluginLibraryFileNameConvention("dinrail-interop");

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
    return nativePluginConvention.libraryStem(deviceName);
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

std::optional<std::string> findNativeDevicePluginLibrary(const std::string& deviceName)
{
    const auto fileNames = nativePluginConvention.fileNameCandidates(deviceName);

    for (const auto& directory : getPluginSearchPaths())
    {
        for (const auto& fileName : fileNames)
        {
            const std::filesystem::path candidate = directory / fileName;
            std::error_code ec;
            if (std::filesystem::is_regular_file(candidate, ec))
            {
                return candidate.string();
            }
        }
    }

    return std::nullopt;
}

std::string getSharedlibppLibraryNameFromInteropName(const std::string& interopName)
{
    return interopPluginConvention.libraryStem(interopName);
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
                = nativePluginConvention.pluginNameFromFileName(entry.path().filename().string());
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

#if defined(DINRAIL_ENABLE_INTEROP_PLUGINS)

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
                = interopPluginConvention.pluginNameFromFileName(entry.path().filename().string());
            if (!name.empty())
            {
                pluginsByName.emplace(name, entry.path().string());
            }
        }
    }
#endif

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

        // Load the exact file found during discovery. In particular, CMake MODULE
        // libraries use the .so suffix on macOS, while sharedlibpp expands a bare
        // library name to .dylib only on that platform.
        bool ok = factory.open(interopPluginInfo.location.c_str(), factoryName.c_str());
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
