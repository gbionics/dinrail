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
 * @brief Build the shared library name for a dinrail device plugin.
 * @param deviceName Device identifier (for example, "fakeMotionControl").
 * @return Library stem in the form "dinrail-device-<deviceName>".
 */
std::string getSharedlibppLibraryNameFromDeviceName(const std::string& deviceName);

/**
 * @brief Build the sharedlibpp factory symbol name for a dinrail device plugin.
 * @param deviceName Device identifier (for example, "fakeMotionControl").
 * @return Factory symbol in the form "dinrail_device_<deviceName>", with dashes converted to
 * underscores.
 */
std::string getSharedlibppFactoryNameFromDeviceName(const std::string& deviceName);

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
