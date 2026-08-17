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
3. a translation supplied by any runtime-loaded interop plugin.

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

### Translating foreign interfaces

An interop plugin can additionally implement `IInterfaceTranslationProvider`
and override `createInterfaceTranslation()` to bridge a foreign interface to a
requested dinrail interface. The translation object owns the adapter and returns
its type-erased interface pointer:

~~~cpp
#include <dinrail/IInterfaceTranslationProvider.h>

class TranslatingInterop final : public dinrail::IInteropPlugin,
                                 public dinrail::IInterfaceTranslationProvider
{
public:
    std::unique_ptr<dinrail::IInterfaceTranslation>
    createInterfaceTranslation(dinrail::IDevice& device,
                               const std::type_info& requested) override;

    // Implement the IInteropPlugin methods as usual.
};

class MyTranslation final : public dinrail::IInterfaceTranslation,
                            public dinrail::IMyInterface
{
public:
    explicit MyTranslation(ForeignInterface& source) : m_source(source) {}

    void* getInterface() override
    {
        return static_cast<dinrail::IMyInterface*>(this);
    }

    // Implement IMyInterface by forwarding/converting calls to m_source.

private:
    ForeignInterface& m_source;
};

std::unique_ptr<dinrail::IInterfaceTranslation>
TranslatingInterop::createInterfaceTranslation(dinrail::IDevice& device,
                                               const std::type_info& requested)
{
    if (requested != typeid(dinrail::IMyInterface))
    {
        return nullptr;
    }

    // Resolve ForeignInterface from device and return MyTranslation on success.
    return nullptr;
}
~~~

`Device` caches successful translations by requested type. The translated
interface pointer stays stable until the device is closed or opened again, and
the translation is destroyed before its source device.
