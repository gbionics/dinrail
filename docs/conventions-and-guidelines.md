# Conventions and Guidelines

This document describes guidelines and conventions used by dinrail interfaces plugins, and should be observed by both built-in dinrail code and custom code that build on top of dinrail.

## Interfaces

Public `dinrail` interfaces should always declare a virtual destructor in the header and define it out-of-line in a `.cpp` file linked into the library. Do not use inline defaulted destructors such as `virtual ~IFoo() = default;` in public interface headers.

This helps to avoid known RTTI and `dynamic_cast` failures that can happen across shared-library boundaries on some toolchain and visibility combinations, especially with `libc++`-based platforms, see for example:
* https://github.com/DOCGroup/bugzilla/issues/2262
* https://github.com/DOCGroup/bugzilla/issues/2143
* https://github.com/llvm/llvm-project/issues/72464
* https://github.com/llvm/llvm-project/issues/36746
* https://github.com/SOCI/soci/issues/913
* https://github.com/android/ndk/issues/1075
* https://github.com/android/ndk/issues/519

## Device plugins

### Device name

`dinrail` devices should always be named following `snake_case`, unless a different name is required to provide drop-in compatibility with legacy devices, for example YARP devices. The class implementing the device should be named according to the `CamelCase` name matching the `snake_case` name of the device.

### Device parameters

`dinrail` devices should always name their parameters using `snake_case`, unless a different style is required to provide drop-in compatibility for existing legacy devices.
