# From `dinrail::Device` to `yarp::dev::PolyDriver`

[Back to the YARP migration guide](yarp-migration.md).

`dinrail::YarpDeviceFromDinrail` is a `yarp::dev::DeviceDriver` template that
owns a native `dinrail::Device` and exposes an explicit list of YARP interface
adapters. For example, the `dr_battery_fake` YARP wrapper is defined as:

```cpp
#include <dinrail/YarpBatteryAdapters.h>
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

The wrapper opens the corresponding native dinrail device and presents the
adapters selected in its type as YARP interfaces. If the native device or a
required interface is unavailable, opening fails.
