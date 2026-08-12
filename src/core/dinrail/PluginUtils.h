// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_PLUGINUTILS_H
#define DINRAIL_PLUGINUTILS_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace dinrail
{

/**
 * @brief Describes a discoverable device.
 */
struct DeviceInfo
{
    std::string name; ///< Device name to pass as the `device` parameter.
    std::string location; ///< Where the device comes from (e.g. a library path).
};

/**
 * @brief Describes an available interop plugin.
 */
struct InteropPluginInfo
{
    std::string name; ///< Interop plugin name (e.g. "yarp").
    std::string location; ///< Interop plugin library path.
};

/**
 * @brief The devices an interop plugin can open.
 */
struct InteropDevices
{
    InteropPluginInfo interopPlugin; ///< Interop plugin metadata.
    std::vector<DeviceInfo> devices; ///< Devices it can open.
};

/**
 * @brief Combined native and interop-provided device discovery results.
 */
struct AvailableDevices
{
    std::vector<DeviceInfo> nativeDevices; ///< Native dinrail device plugins.
    std::vector<InteropDevices> interopDevices; ///< Devices grouped by interop plugin.
};

/**
 * @brief Build the shared library name for a dinrail device plugin.
 * @param deviceName Device identifier (for example, "dr_controlboard_fake").
 * @return Library stem in the form "dinrail-device-<deviceName>".
 */
std::string getSharedlibppLibraryNameFromDeviceName(const std::string& deviceName);

/**
 * @brief Build the sharedlibpp factory symbol name for a dinrail device plugin.
 * @param deviceName Device identifier (for example, "dr_controlboard_fake").
 * @return Factory symbol in the form "dinrail_device_<deviceName>", with dashes converted to
 * underscores.
 */
std::string getSharedlibppFactoryNameFromDeviceName(const std::string& deviceName);

/**
 * @brief Build the shared library name for a dinrail interop plugin.
 * @param interopName Interop identifier (for example, "yarp").
 * @return Library stem in the form "dinrail-interop-<interopName>".
 */
std::string getSharedlibppLibraryNameFromInteropName(const std::string& interopName);

/**
 * @brief Build the sharedlibpp factory symbol name for a dinrail interop plugin.
 * @param interopName Interop identifier (for example, "yarp").
 * @return Factory symbol in the form "dinrail_interop_<interopName>", with dashes converted to
 * underscores.
 */
std::string getSharedlibppFactoryNameFromInteropName(const std::string& interopName);

/**
 * @brief Enumerate native dinrail device plugins available on plugin search paths.
 *
 * @return Sorted, de-duplicated list of native devices with their library location.
 */
std::vector<DeviceInfo> getAvailableNativeDevices();

/**
 * @brief Enumerate interop plugins available on plugin search paths.
 *
 * @return Sorted, de-duplicated list of interop plugins with their library location.
 */
std::vector<InteropPluginInfo> getAvailableInteropPlugins();

/**
 * @brief Enumerate the interop plugins available on the plugin search paths.
 *
 * Scans the directories returned by getPluginSearchPaths() for shared libraries
 * whose name matches the interop plugin convention (`dinrail-interop-<name>`)
 * and returns the extracted `<name>` values.
 *
 * @return Sorted, de-duplicated list of available interop plugin names.
 */
std::vector<std::string> getAvailableInteropPluginNames();

/**
 * @brief Enumerate devices exposed by available interop plugins.
 *
 * @return Devices grouped by interop plugin.
 */
std::vector<InteropDevices> getAvailableInteropDevices();

/**
 * @brief Enumerate all available devices, including interop-provided ones.
 *
 * @return Native devices and interop devices in one struct.
 */
AvailableDevices getAvailableDevices();

/**
 * @brief Resolve the directory that contains the loaded dinrail shared library.
 *
 * This is used to discover plugin directories relative to the dinrail runtime library location,
 * regardless of the executable path.
 *
 * @return Directory path containing the loaded dinrail library, or empty optional if it cannot be
 * resolved.
 */
std::optional<std::string> getPathOfDinrailSharedLibrary();

/**
 * @brief Compute candidate plugin search paths for dinrail device plugins.
 *
 * The returned list contains, in order:
 * 1) The directory of the loaded dinrail shared library (if resolvable).
 * 2) Additional entries from the `DINRAIL_PLUGIN_PATH` environment variable.
 *
 * @return Ordered list of candidate plugin directories.
 */
std::vector<std::filesystem::path> getPluginSearchPaths();

} // namespace dinrail

#endif // DINRAIL_PLUGINUTILS_H
