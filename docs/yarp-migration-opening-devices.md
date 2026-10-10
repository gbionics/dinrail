# From `yarp::dev::PolyDriver` to `dinrail::Device`

[Back to the YARP migration guide](yarp-migration.md).

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

## Requirements

You can check which interop plugins are available (and therefore which ecosystems
`dinrail::Device` can open devices from) with `dinrail interop --list`.

To enumerate native devices and devices reported by interop plugins, use
`dinrail dev --list`.
