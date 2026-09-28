# Migrating Code from YARP Devices to dinrail Devices

## From `yarp::os::Property` to `dinrail::Parameters`

### Scalar values

As long as you are retrieving scalar parameters, you can use similar APIs:

- `yarp::os::Property` -> `dinrail::Parameters`
- `property.find("k")` -> `params.find("k")`
- `property.check("k")` -> `params.check("k")`
- `property.findGroup("G")` -> `params.findGroup("G")`
- `property.put("k", v)` -> `params.put("k", v)` or `params.setParameter("k", v)`

## Nested groups

Nested groups are handled with:

- `params.addGroup("GROUP")`
- `params.findGroup("GROUP")`

## Vector values

If instead you are reading and writing vector values, you must port your code to use `setParameter(...)` and `getParameter(...)` with `dinrail::VectorProxy`.

Unlike scalar values, vector values are not accessed via `find("k")` returning a `dinrail::Value`. The intended API is:

- write with `params.setParameter("k", container)`
- read with `params.getParameter("k", container)`

For example, code such as:

```cpp
yarp::os::Bottle* gains = property.find("gains").asList();
std::vector<double> values;

for (int i = 0; i < gains->size(); ++i)
{
	values.push_back(gains->get(i).asFloat64());
}
```

can be migrated to:

```cpp
#include <dinrail/Parameters.h>

std::vector<double> values;

const bool ok = params.getParameter("gains", values);
```

Similarly, writing a vector becomes:

```cpp
std::vector<std::string> jointNames{"shoulder", "elbow", "wrist"};

params.setParameter("joint_names", jointNames);
```

If your destination container is resizable, `getParameter(...)` resizes it as needed. If it is not resizable, such as `std::array`, the call succeeds only if the stored vector size already matches the destination size.

For more details and more examples, see [parameters.md](parameters.md).

Note that while vector values of heterogenous types are supported in `yarp::os::Property`, to keep the complexity low these are not supported in `dinrail::Parameters`.

## From YARP Logging to spdlog

`dinrail` does not re-define its own logging mechanism, but it mostly relies on [spdlog](https://github.com/gabime/spdlog) for logging.

When migrating code from YARP devices, replace YARP logging includes and macros with `spdlog` calls.

### Includes

Remove:

```cpp
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
```

Add:

```cpp
#include <spdlog/spdlog.h>
```

### Components

In YARP, custom logging components are typically defined with:

```cpp
YARP_DECLARE_LOG_COMPONENT(CONTROLBOARD)
YARP_LOG_COMPONENT(CONTROLBOARD, "dinrail.controlboard")
```

In spdlog, create one logger per component name (for example `yarp.device.controlboard`) and
reuse it everywhere directly in the implementation code:

~~~cxx
#include <dinrail/SpdlogHelpers.h>

spdlog::logger& controlBoardLogger()
{
    static const auto instance = dinrail::createOrGetLogger("dinrail.controlboard");
    return *instance;
}
~~~

In the snippet the logger is created with a free function, but in case of components implemented as classes, `controlBoardLogger()` can just be methods of the class.The only important aspect is to make sure that the `static const auto instance = createLogger()` is not put in an inline/header-only method or function. A common workflow is to put this logger functions in a dedicated `.h` and corresponding `.cpp` files.

The resulting logger can be invoked with `controlBoardLogger()`.

The `dinrail::createOrGetLogger` function from `dinrail/SpdlogHelpers.h` is a short self-contained function, so if you need to migrate YARP logging code that does not depend on dinrail, just copy the function in your codebase.

### Macro mapping

Use the following mapping:

- `yError(...)` -> `spdlog::error(...)`
- `yWarning(...)` -> `spdlog::warn(...)`
- `yInfo(...)` -> `spdlog::info(...)`
- `yDebug(...)` -> `spdlog::debug(...)`
- `yCError(COMP, ...)` -> `controlBoardLogger().error(...)`
- `yCWarning(COMP, ...)` -> `controlBoardLogger().warn(...)`
- `yCInfo(COMP, ...)` -> `controlBoardLogger().info(...)`
- `yCDebug(COMP, ...)` -> `controlBoardLogger().debug(...)`
- `yCAssert(COMP, cond)` -> `if (!(cond)) { controlBoardLogger().error(...); assert(cond); }`

#### Throttle macros

For throttled logging (at most once every `period` seconds), use:

- `yErrorThrottle(period, ...)` -> `DINRAIL_ERROR_THROTTLE(period, spdlog::default_logger(), ...)`
- `yWarningThrottle(period, ...)` -> `DINRAIL_WARN_THROTTLE(period, spdlog::default_logger(), ...)`
- `yInfoThrottle(period, ...)` -> `DINRAIL_INFO_THROTTLE(period, spdlog::default_logger(), ...)`
- `yDebugThrottle(period, ...)` -> `DINRAIL_DEBUG_THROTTLE(period, spdlog::default_logger(), ...)`
- `yCErrorThrottle(COMP, period, ...)` -> `DINRAIL_ERROR_THROTTLE(period, controlBoardLogger(), ...)`
- `yCWarningThrottle(COMP, period, ...)` -> `DINRAIL_WARN_THROTTLE(period, controlBoardLogger(), ...)`
- `yCInfoThrottle(COMP, period, ...)` -> `DINRAIL_INFO_THROTTLE(period, controlBoardLogger(), ...)`
- `yCDebugThrottle(COMP, period, ...)` -> `DINRAIL_DEBUG_THROTTLE(period, controlBoardLogger(), ...)`

These throttle macros are defined in `dinrail/SpdlogHelpers.h` and ensure that a message is logged at most once every `period` seconds.

### Examples

Format-style message:

```cpp
// YARP
yCWarning(CONTROLBOARD, "Joint %d is saturated", joint);

// spdlog
controlBoardLogger().warn("Joint {} is saturated", joint);
```

Stream-style message:

```cpp
// YARP
yCError(CONTROLBOARDREMAPPER) << "Attach failed for device " << deviceKey;

// spdlog
controlBoardLogger().error("Attach failed for device {}", deviceKey);
```

Throttled logging (at most once every 1.0 second):

```cpp
// YARP
yCWarningThrottle(CONTROLBOARD, 1.0, "Sensor reading out of range: %f", reading);

// spdlog (include dinrail/SpdlogHelpers.h)
DINRAIL_WARN_THROTTLE(1.0, controlBoardLogger(), "Sensor reading out of range: {}", reading);
```

### Notes

- Prefer `{}` placeholders over `%` placeholders.
- Prefer dedicated named loggers (for example `dinrail.controlboard`) instead of manually prefixing message text with component names.
- Ensure the target links against `spdlog::spdlog` in CMake.
- By default, the YARP logging system prints `debug` outputs. This are hidden by default in spdlog. To see debug prints, set the `SPDLOG_LEVEL` to `debug` .

## From `yarp::dev::PolyDriver` to `dinrail::Device`

`dinrail::Device` can open both native dinrail devices and YARP devices. When you
open a device whose name does not match a native `dinrail-device-<name>` plugin,
dinrail falls back to its interop plugins; the bundled `dinrail-interop-yarp`
plugin opens the device as a `yarp::dev::PolyDriver` behind the scenes and wraps
it as a `dinrail::IDevice`. This means you can keep using the same YARP device
names and parameters.

For example, opening a YARP device directly:

```cpp
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IAxisInfo.h>
#include <yarp/os/Property.h>

yarp::os::Property options;
options.put("device", "fakeMotionControl");
options.addGroup("GENERAL").put("Joints", 3);

yarp::dev::PolyDriver driver;
driver.open(options);

yarp::dev::IAxisInfo* axisInfo = nullptr;
driver.view(axisInfo);
```

can be migrated to:

```cpp
#include <dinrail/Device.h>
#include <dinrail/Parameters.h>
#include <yarp/dev/IAxisInfo.h>

dinrail::Parameters options;
options.put("device", "fakeMotionControl");
options.addGroup("GENERAL").put("Joints", 3);

dinrail::Device device;
device.open(options);

// Native yarp::dev::* interfaces of the wrapped device are resolved by view().
yarp::dev::IAxisInfo* axisInfo = nullptr;
device.view(axisInfo);
```

The device name and parameters are the same; dinrail converts
`dinrail::Parameters` to `yarp::os::Property` automatically when delegating to the
YARP interop plugin.

`dinrail::Device::view<T>()` resolves both dinrail interfaces implemented by the
device and the native `yarp::dev::*` interfaces of the wrapped YARP device, so
existing YARP interface code keeps working after the migration.

### Requirements

You can check which interop plugins are available (and therefore which ecosystems
`dinrail::Device` can open devices from) with `dinrail interop --list`.

To enumerate native devices and devices reported by interop plugins, use
`dinrail dev --list`.

## From `dinrail::Device` to `yarp::dev::PolyDriver`

`dinrail::YarpDeviceFromDinrail` is a `yarp::dev::DeviceDriver` template that
owns a native `dinrail::Device` and exposes an explicit list of YARP interface
adapters. For example, the `dr_battery_fake` YARP wrapper is defined as:

```cpp
#include <dinrail/BatteryAdapters.h>
#include <dinrail/YarpDeviceFromDinrail.h>

using DrBatteryFakeYarp =
    dinrail::YarpDeviceFromDinrail<
        dinrail::InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>>;
```

The wrapper can then be registered as a YARP plugin with the supplied CMake
helper:

```cmake
dinrail_add_yarp_device(dr_battery_fake DrBatteryFakeYarp DrBatteryFakeYarp.h)
```

Once installed, the native dinrail device can be opened through
`yarp::dev::PolyDriver` under the same name:

```cpp
#include <yarp/dev/IBattery.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Property.h>

yarp::os::Property config;
config.put("device", "dr_battery_fake");

yarp::dev::PolyDriver driver(config);
yarp::dev::IBattery* battery = nullptr;
driver.view(battery);
```

The wrapper converts the YARP configuration to `dinrail::Parameters` and sets
`dinrail_device_type` to `dinrail`, ensuring that only the native device plugin
is considered and avoiding recursive interop fallback. It then opens the device
and binds every selected adapter to an interface returned by
`dinrail::Device::view()`. Opening fails if the native device cannot be opened or
if any selected adapter source is unavailable.

Adapters are selected explicitly: interfaces not listed in the wrapper type are
not exposed through YARP. The selected adapters must be default constructible,
concrete, and provide each target interface only once; these constraints are
checked at compile time.

The wrapper unbinds its adapters before closing the native device. When using
the concrete wrapper directly, `device()` returns the owned `dinrail::Device`,
which can be used to access native-only interfaces such as simulation controls.
The wrapper links the adapter library but does not link the native device
implementation; the native device remains a separate runtime plugin.
