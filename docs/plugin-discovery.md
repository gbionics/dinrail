# dinrail Plugin Discovery

This page documents how dinrail discovers device plugins at runtime.

## Overview

When opening a device with `dinrail::Device`, the library computes a list of candidate directories and asks `sharedlibpp` to search there for the plugin shared library.

For a device name `X`:

- plugin library stem: `dinrail-device-X`
- factory symbol: `dinrail_device_X` (with `-` in the device name converted to `_`)

The logic lives in [src/core/dinrail/PluginUtils.h](../src/core/dinrail/PluginUtils.h) and [src/core/dinrail/PluginUtils.cpp](../src/core/dinrail/PluginUtils.cpp).

## Search Path Resolution

`dinrail::getPluginSearchPaths()` returns candidate directories in this order:

1. The directory containing the loaded `dinrail` .so shared library (on Linux and macOS), or `.dll` on Windows  (if it can be resolved).
2. Entries from `DINRAIL_PLUGIN_PATH` (split with `:` on Linux/macOS and `;` on Windows).

`dinrail::Device::open(...)` extends the `sharedlibpp` search path with all returned directories.

The `dinrail` CLI (`dev --show-search-path` and `dev --list`) uses the same `getPluginSearchPaths()` function, so CLI output and runtime loading behavior are aligned.

This logic should ensure that as long as you install dinrail devices in the same install prefix of `dinrail` itself, all the plugins will be found without setting any additional 
environmental variable, otherwise if the plugin install prefix is different, you just need to add the install location of plugins to `DINRAIL_PLUGIN_PATH`.

