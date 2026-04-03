# Naming Conventions

This document describes the naming conventions used by dinrail plugins, and should be observed by both built-in dinrail plugins and custom dinrail plugins.

## Device plugins

### Device name

`dinrail` devices should always be named following `CamelCase`, unless a different name is required to provide drop-in compatibility with legacy devices, for example YARP devices.

### Device parameters

`dinrail` devices should always name their parameters using `snake_case`, unless a different style is required to provide drop-in compatibility for existing legacy devices.
