# YARP to dinrail Migration

This document explains how to migrate C++ code from YARP devices to dinrail devices.

## Core Concepts

- YARP uses `yarp::dev::PolyDriver` and `yarp::os::Property`.
- dinrail uses `dinrail::Device` and `dinrail::Parameters`.
- dinrail keeps interface-driven usage (`view` + interface pointers), but configuration now uses a dedicated parameter container instead of trying to fully mimic `yarp::os::Property`.

## Typical Migration Steps

1. Replace `yarp::dev::PolyDriver` with `dinrail::Device`.
2. Replace `yarp::os::Property` with `dinrail::Parameters`.
3. Keep using `put(...)` for parameters.
4. Keep using `addGroup(...)` for nested parameter groups.
5. Keep the same interface flow (`view`, then method calls), using dinrail interfaces.

## Configuration Snippet Migration

### Before (YARP)

```cpp
#include <yarp/os/Property.h>

yarp::os::Property opts;
opts.put("device", "fakeMotionControl");

yarp::os::Property& general = opts.addGroup("GENERAL");
general.put("Joints", 6);
```

### After (dinrail)

```cpp
#include <dinrail/Parameters.h>

dinrail::Parameters opts;
opts.put("device", "fakeMotionControl");

dinrail::Parameters& general = opts.addGroup("GENERAL");
general.put("Joints", 6);
```

## Full Device Opening Pattern

```cpp
#include <dinrail/Device.h>
#include <dinrail/Parameters.h>

int main()
{
    dinrail::Parameters opts;
    opts.put("device", "fakeMotionControl");
    opts.addGroup("GENERAL").put("Joints", 6);

    if (!opts.check("device"))
    {
        return 1;
    }

    dinrail::Device device;
    if (!device.open(opts))
    {
        return 1;
    }

    // Acquire interfaces with device.view(...)
    // ...

    device.close();
    return 0;
}
```

## Notes

- Parameter values are typed. Use `getString`, `getInt`, `getDouble`, and `getBool` when reading configuration in device code.
- Groups are hierarchical and can be nested via repeated calls to `addGroup(...)`.
- Existing YARP-based plugins can still be loaded through dinrail compatibility layers.
