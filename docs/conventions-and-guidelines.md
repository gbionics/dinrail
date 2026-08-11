# Conventions and Guidelines

This document describes guidelines and conventions used by dinrail interfaces plugins, and should be observed by both built-in dinrail code and custom code that build on top of dinrail.

## Device plugins

### Device name

`dinrail` devices should always be named following `snake_case`, unless a different name is required to provide drop-in compatibility with legacy devices, for example YARP devices. The class implementing the device should be named according to the `CamelCase` name matching the `snake_case` name of the device.

### Device parameters

`dinrail` devices should always name their parameters using `snake_case`, unless a different style is required to provide drop-in compatibility for existing legacy devices.
