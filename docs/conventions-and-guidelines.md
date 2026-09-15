# Conventions and Guidelines

This document describes guidelines and conventions used by dinrail interfaces and plugins, and should be observed by both built-in dinrail code and custom code that builds on top of dinrail.

## Interfaces

### Virtual destructors

Public `dinrail` interfaces should always declare a virtual destructor in the header and define it out-of-line in a `.cpp` file linked into the library. Do not use inline defaulted destructors such as `virtual ~IFoo() = default;` in public interface headers.

This helps to avoid known RTTI and `dynamic_cast` failures that can happen across shared-library boundaries on some toolchain and visibility combinations, especially with `libc++`-based platforms, see for example:
* https://github.com/DOCGroup/bugzilla/issues/2262
* https://github.com/DOCGroup/bugzilla/issues/2143
* https://github.com/llvm/llvm-project/issues/72464
* https://github.com/llvm/llvm-project/issues/36746
* https://github.com/SOCI/soci/issues/913
* https://github.com/android/ndk/issues/1075
* https://github.com/android/ndk/issues/519

### Simulation interfaces

For each interface that it is supposed to read or write data that changes over time, an interface named `<InterfaceNamed>Simulation is expected to be available.

The role of this simulation interface is meant to permit simple simulation to be implemented on top of simple `fake` devices.

For example, if the interface is defined as:

~~~cpp
class IPreciselyTimed
{
public:
    virtual ~IPreciselyTimed();
    virtual Stamp getLastInputStamp() = 0;
};
~~~

the related simulation interface would be defined as:

~~~cpp
class IPreciselyTimedSimulation
{
public:
    virtual ~IPreciselyTimedSimulation();
    virtual void setLastInputStamp(const Stamp& stamp) = 0;
};
~~~

As the `Simulation` interfaces are only meant to be used to implement a simulator, they are not handled by remappers, network wrapper server and network wrapper clients.

## Device plugins

### Device name

`dinrail` devices should always be named following `snake_case`, unless a different name is required to provide drop-in compatibility with legacy devices, for example YARP devices. The class implementing the device should be named according to the `CamelCase` name matching the `snake_case` name of the device.

### Device parameters

`dinrail` devices should always name their parameters using `snake_case`, unless a different style is required to provide drop-in compatibility for existing legacy devices.

### Downstream devices

`dinrail` devices that are not maintained in the `dinrail` repository may have to support multiple versions of the `dinrail` interfaces. According to the dinrail versioning policy, API changes may also occur in minor releases.

To keep compile-time compatibility with different dinrail versions, downstream code maintainers are encouraged to use the `DINRAIL_VERSION_AT_LEAST` macro defined in `<dinrail/Version.h>`. For example, code supporting both dinrail 0.1 and an API introduced in dinrail 0.2 can use:

~~~cpp
#include <dinrail/Version.h>

#if DINRAIL_VERSION_AT_LEAST(0, 2, 0)
// Use the API available since dinrail 0.2.0.
#else
// Use the compatibility implementation for dinrail 0.1.x.
#endif
~~~
