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

### Notes

- Prefer `{}` placeholders over `%` placeholders.
- Prefer dedicated named loggers (for example `dinrail.controlboard`) instead of manually prefixing message text with component names.
- Ensure the target links against `spdlog::spdlog` in CMake.
- By default, the YARP logging system prints `debug` outputs. This are hidden by default in spdlog. To see debug prints, set the `SPDLOG_LEVEL` to `debug` .
