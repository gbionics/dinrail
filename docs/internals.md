# Plugin types

`dinrail` heavily relies on [Factory Pattern](https://en.wikipedia.org/wiki/Factory_method_pattern) to implement its functionalities, mainly relying on the [sharedlibpp](https://github.com/gbionics/sharedlibpp) library to implement it using dynamic loadable libraries.

In the following, we describe the different plugin types supported by `dinrail`.

* [dinrail device](#dinrail-device)
* [dinrail compatibility layer](#dinrail-compatibility-layer)


## Dinrail Device

This is the basic abstraction unit of dinrail. A device is a class that:

1. inherits from `dinrail::IDevice` 
2. is compiled into a shared library named **`dinrail-device-<devicename>`**,
3. exposes a factory symbol named **`dinrail_device_<devicename>`** (hyphens replaced by underscores) via the `SHLIBPP_DEFINE_SHARED_SUBCLASS` macro.

The three pure-virtual methods that every device must implement are:

| Method | Description |
|---|---|
| `bool open(const Parameters& config)` | Configure and start the device. |
| `bool close()` | Stop the device and release its resources. |
| `IDevice* allocateInstance() const` | Return a freshly heap-allocated copy (used by the plugin loader). |

### Naming convention

| Artifact | Pattern | Example |
|---|---|---|
| Shared library filename | `dinrail-device-<name>` | `libdinrail-device-fakeMotionControl.so` |
| sharedlibpp factory symbol | `dinrail_device_<name>` | `dinrail_device_fakeMotionControl` |


## Dinrail Compatibility Layer

A compatibility layer is a plugin that teaches `dinrail` how to open devices from a foreign framework (e.g. YARP) as if they were native dinrail devices.  
It is defined as a class that inherits from `dinrail::ICompatibilityLayer` and is found in a shared library named **`dinrail-compat-<name>`**.

The three pure-virtual methods that every compatibility layer must implement are:

| Method | Description |
|---|---|
| `std::unique_ptr<IDevice> createDevice(const Parameters& config)` | Instantiate and open a foreign device, wrapping it in a `dinrail::IDevice`. |
| `ICompatibilityLayer* allocateInstance() const` | Return a freshly heap-allocated copy (used by the plugin loader). |
| `void registerInterfaceAdapters()` | Register adapter factories in `AdapterRegistry` so that `Device::view<T>()` can bridge foreign interfaces. |


The CMake target must be a `MODULE` library named `dinrail-compat-myframework` and should link against `dinrail::dinrail`.
