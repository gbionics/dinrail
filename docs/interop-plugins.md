# Interoperability plugins

Interoperability plugins give dinrail drop-in compatibility with foreign device systems (such as YARP) without directly depending on YARP or another library. In regular use of dinrail,
you do not need to write an interop plugin, but these notes are provided for the rare case when a new interop plugin needs to be written.

### Writing an interop plugin

An interop plugin implements `dinrail::IInteropPlugin` and opens a foreign device
from the configuration:

~~~cpp
#include <dinrail/IInteropPlugin.h>
#include <sharedlibpp/SharedLibraryClassApi.h>

class MyInterop final : public dinrail::IInteropPlugin
{
public:
    std::unique_ptr<dinrail::IDevice> createDevice(const dinrail::Parameters& config) override
    {
        static_cast<void>(config);
        return nullptr;
    }

    std::vector<dinrail::DeviceInfo> listDevices() const override
    {
        return {};
    }
};

// Registers the factory symbol `dinrail_interop_myinterop`; the library must be
// named `dinrail-interop-myinterop`.
SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_interop_myinterop, MyInterop, dinrail::IInteropPlugin)
~~~

### Adding new interfaces to `view()`

`dinrail::Device::view<T>()` resolves an interface by, in order:

1. a direct cast of the device to `T`;
2. `viewInterface(typeid(T))` if the device implements `dinrail::IInterfaceView`;
3. an adapter registered by an available interop plugin.

A device that wraps a foreign implementation can expose arbitrary interfaces by
implementing `dinrail::IInterfaceView`. For example, the YARP interop plugin
wraps a `yarp::dev::PolyDriver` and resolves any native `yarp::dev::*` interface
with a single `runtimeDynamicCast` in `viewInterface`:

~~~cpp
class MyDevice final : public dinrail::IDevice, public dinrail::IInterfaceView
{
public:
    void* viewInterface(const std::type_info& interfaceType) override
    {
        if (interfaceType == typeid(IMyCustomInterface))
        {
            return static_cast<IMyCustomInterface*>(&m_customInterface);
        }
        return nullptr;
    }
    // ...
};
~~~

With this in place, `device.view<IMyCustomInterface>(ptr)` returns the interface
provided by the device.

### Interface adapters

An interface adapter lets `Device::view<Target>()` provide a requested `Target`
interface when the device exposes only a `Source` interface. To define one,
derive from `dinrail::InterfaceAdapterBase<Target, Source>` and implement the
`Target` methods. The base class stores a reference to the `Source` object; its
protected `source()` method returns that object so the adapter can call its
methods and convert their inputs or results as needed.

An interop plugin registers the adapter from its `registerInterfaceAdapters()`
override by calling `registry.add<Target, Source, Adapter>()`, where `Adapter` is
the concrete adapter class.

Adapters provided by dinrail conventionally specialize
`dinrail::InterfaceAdapter<Target, Source>`. This keeps the fully qualified target
and source interface types visible wherever an adapter is used. It is only a
naming convention: a non-templated class works too, provided that it derives
from `dinrail::InterfaceAdapterBase<Target, Source>`.

For example, the YARP plugin registers battery adapters in both directions:

```cpp
void YarpInteropPlugin::registerInterfaceAdapters(InterfaceAdapterRegistry& registry)
{
    // ...
    registry.add<dinrail::IBattery, yarp::dev::IBattery,
                 InterfaceAdapter<dinrail::IBattery, yarp::dev::IBattery>>();
    registry.add<yarp::dev::IBattery, dinrail::IBattery,
                 InterfaceAdapter<yarp::dev::IBattery, dinrail::IBattery>>();
    // ...
}
```

The specializations in `YarpBatteryAdapters.h` convert operation results between
`dinrail::Status` and YARP's return values, and convert battery status values.
With the YARP interop plugin available on the plugin search path, a native
battery can expose the YARP interface through the ordinary device handle:

```cpp
dinrail::Parameters config;
config.put("device", "dr_battery_fake");
config.put("dinrail_device_type", "dinrail");
dinrail::Device device;
if (device.open(config))
{
    yarp::dev::IBattery* battery = nullptr;
    if (device.view(battery))
    {
        double voltage = 0.0;
        battery->getBatteryVoltage(voltage);
    }
}
```

