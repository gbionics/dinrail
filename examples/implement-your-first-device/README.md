# Implement Your First Device

This example shows how to implement a minimal dinrail device plugin and how to expose both:

- a built-in dinrail interface (`dinrail::IAxisInfo`), and
- a project-specific custom interface (`myexample::MyInterface`).

## Files

- `MyInterface.h`: custom interface definition.
- `MyDevice.h` / `MyDevice.cpp`: device implementation and plugin registration.
- `main.cpp`: runtime usage through `dinrail::Device::view<T>()`.
- `CMakeLists.txt`: plugin and executable targets.

## 1. Define a Custom Interface

`MyInterface.h` defines the custom API:

- `bool setGain(double gain)`
- `bool getGain(double& gain)`

The interface is plain C++ and independent from dinrail internals.

## 2. Implement the Device Plugin

`MyDevice` inherits from:

- `dinrail::IDevice` (required for all dinrail devices),
- `dinrail::IAxisInfo` (standard interface),
- `myexample::MyInterface` (custom interface).

Important points in `MyDevice.cpp`:

- `open(const dinrail::Parameters&)` reads configuration (`number_of_joints`, `joint_names`, `gain`).
- `getAxes/getAxisName/getJointType` implement `dinrail::IAxisInfo`.
- `setGain/getGain` implement `myexample::MyInterface`.
- `SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_device_myDevice, MyDevice, dinrail::IDevice)` registers the plugin factory.

## 3. Use the Device and the Custom Interface

In `main.cpp`:

1. Build a `dinrail::Parameters` configuration.
2. Open `myDevice` through `dinrail::Device`.
3. Query `dinrail::IAxisInfo` with `view<dinrail::IAxisInfo>()`.
4. Query the custom interface with `view<myexample::MyInterface>()`.
5. Call `getGain()` / `setGain()` on the custom interface.

Because `MyDevice` directly inherits `myexample::MyInterface`, `view<myexample::MyInterface>()` works without any compatibility layer.

## Build Notes

This example is included when the project is configured with `DINRAIL_BUILD_EXAMPLES=ON`.

The CMake target names are:

- plugin: `dinrail-device-myDevice`
- executable: `implement-your-first-device`
