// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/PluginUtils.h>

#include <cstdlib>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace dinrail
{

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

std::vector<std::filesystem::path>
getPluginSearchPaths()
{
    std::vector<std::filesystem::path> searchPaths;

    const std::optional<std::string> libPath = getPathOfDinrailSharedLibrary();
    if (libPath.has_value())
    {
        searchPaths.emplace_back(libPath.value());
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
