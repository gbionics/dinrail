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

    void registerInterfaceAdapters(
        dinrail::InterfaceAdapterRegistry& registry) override
    {
        static_cast<void>(registry);
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
3. an adapter supplied by a runtime-loaded interop plugin.

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

### Adapting foreign interfaces

An interop plugin registers factories in the runtime adapter registry to bridge
foreign and dinrail interfaces. Plugins without adapters leave the
registry unchanged:

~~~cpp
#include <dinrail/IInteropPlugin.h>

class MyAdapter final
    : public dinrail::InterfaceAdapter<dinrail::IMyInterface, ForeignInterface>
{
public:
    using InterfaceAdapter::InterfaceAdapter;

    // Implement IMyInterface by forwarding or converting calls to source().
};

class MyInterop final : public dinrail::IInteropPlugin
{
public:
    void registerInterfaceAdapters(
        dinrail::InterfaceAdapterRegistry& registry) override
    {
        registry.add<dinrail::IMyInterface, ForeignInterface, MyAdapter>();
    }

    // Implement IInteropPlugin as usual.
};
~~~

`Device` caches each successful adapter by requested type. The adapted interface
pointer remains stable until the device is closed or opened again,
and its adapter is destroyed before the source device.

The runtime owns one adapter registry keyed by the requested destination
interface (`std::type_index`). Each loaded interop plugin populates it once. A
request looks up only that interface's factories and tries them in registration
order until one succeeds; unknown interfaces return `nullptr` without probing
the device.

To add a YARP adapter, register it in its family's registration function with
`registry.add<dinrail::IMyInterface, yarp::dev::IMyInterface, MyAdapter>()`.
Adapters requiring multiple source interfaces can register a factory using
Adapters should expose each supported source-interface combination through a
separate single-source adapter and register the more specific adapter first.
Dinrail-to-YARP adapters derive from
`dinrail::InterfaceAdapter<yarp::dev::ITarget, dinrail::ISource>` and are
registered alongside their YARP-to-dinrail counterparts in the relevant
adapter family. A YARP wrapper explicitly lists every adapter it exposes;
it never chooses one from the catalog. At `open()`, the wrapper opens a native `dinrail::Device` from the
YARP configuration with `Device::openNative()` and resolves each selected
adapter source through `Device::view()`.
Opening fails when any selected source is unavailable. The wrapper unbinds its
adapters before closing the runtime device.

Adapter implementations live in `dinrail-yarp-conversions`, shared by the
interop plugin and YARP device plugins. A native `dinrail::Device` can therefore
also request `yarp::dev::*` interfaces via `view()` when the YARP interop plugin is
loaded. Successful reverse adapters use the same cache and lifetime rules.


### External adapters and shared interfaces

`dinrail::InterfaceAdapterRegistry` and `dinrail::InterfaceAdapter<Target, Source>`
are public, YARP-independent helpers in dinrail core. External repositories can
provide adapters and register them from their own interop plugins. Deriving from
An adapter-only plugin returns `nullptr` from `createDevice()`, returns an
empty list from `listDevices()`, and performs its registrations in
`registerInterfaceAdapters()`:

```cpp
class MyAdapterPlugin final : public dinrail::IInteropPlugin
{
public:
    std::unique_ptr<dinrail::IDevice>
    createDevice(const dinrail::Parameters&) override
    {
        return nullptr;
    }

    std::vector<dinrail::DeviceInfo> listDevices() const override
    {
        return {};
    }

    void registerInterfaceAdapters(
        dinrail::InterfaceAdapterRegistry& registry) override
    {
        registry.add<dinrail::IMyInterface, ForeignInterface, MyAdapter>();
    }
};
```

A YARP wrapper can choose an external adapter explicitly, bypassing the built-in
catalog:

```cpp
using MyYarpDevice = dinrail::YarpDevice<
    MyChosenAdapter>;
```

If an interface is already independent of YARP, keep the same interface type.
Write a forwarder deriving from `dinrail::InterfaceAdapter<SharedInterface, SharedInterface>`
and select `MyForwarder` directly. Its methods
forward to `source()` without converting arguments or results. This preserves
shared sensor/actuator object pointers as well. YARP's dynamic-cast-based view
requires the wrapper to implement the methods even though it owns the source.

Wrappers fail to compile for abstract or non-default-constructible adapters, or
when more than one adapter provides the same target interface.
