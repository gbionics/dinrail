# dinrail Plugin Discovery

This page documents how dinrail discovers device and interop plugins at runtime.

## Overview

When opening a device with `dinrail::Device`, the library computes a list of candidate directories and asks `sharedlibpp` to search there for plugin shared libraries.

For a device name `X`:

- plugin library stem: `dinrail-device-X`
- factory symbol: `dinrail_device_X` (with `-` in the device name converted to `_`)

If the device plugin cannot be opened, dinrail can try configured interop plugins.

For an interop plugin name `Y`:

- plugin library stem: `dinrail-interop-Y`
- factory symbol: `dinrail_interop_Y` (with `-` in the interop name converted to `_`)

The naming logic lives in [src/core/dinrail/PluginUtils.h](../src/core/dinrail/PluginUtils.h) and [src/core/dinrail/PluginUtils.cpp](../src/core/dinrail/PluginUtils.cpp), while fallback loading is handled by [src/core/dinrail/RuntimeContext.cpp](../src/core/dinrail/RuntimeContext.cpp).

## Interop plugins

An interop plugin lets dinrail open devices provided by another ecosystem (for
example YARP) when no native `dinrail-device-<name>` plugin can be opened.

### Discovery

`dinrail::Device` scans the plugin search paths for `dinrail-interop-<name>`
shared libraries. When a device open cannot be satisfied by a native
`dinrail-device-<name>` plugin, the available interop plugins are tried in
alphabetical order and the first one that returns a device wins.

You can inspect the available interop plugins with the CLI:

~~~
dinrail interop --list
~~~

## Search Path Resolution

`dinrail::getPluginSearchPaths()` returns candidate directories in this order:

1. The directory containing the loaded `dinrail` .so shared library (on Linux and macOS), or `.dll` on Windows  (if it can be resolved).
2. Entries from `DINRAIL_PLUGIN_PATH` (split with `:` on Linux/macOS and `;` on Windows).

`dinrail::Device::open(...)` extends the `sharedlibpp` search path with all returned directories.

The `dinrail` CLI (`dev --show-search-path` and `dev --list`) uses the same `getPluginSearchPaths()` function, so CLI output and runtime loading behavior are aligned.

The `dinrail interop --list` and `dinrail interop --show-search-path` verbs expose interop plugin discovery for debugging.

This logic should ensure that as long as you install dinrail devices in the same install prefix as `dinrail` itself, all the plugins will be found without setting any additional environment
variable. If the plugin install prefix is different, you just need to add the install location of plugins to `DINRAIL_PLUGIN_PATH`.

