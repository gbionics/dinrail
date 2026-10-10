# Migrating Code from YARP Devices to dinrail Devices

* [Parameters](yarp-migration-parameters.md) — Replace `yarp::os::Property` with `dinrail::Parameters`, including scalar values, nested groups, and vectors.
* [Logging](yarp-migration-logging.md) — Replace YARP logging with spdlog, including component loggers, macro mappings, and throttled messages.
* [Opening YARP devices with dinrail](yarp-migration-opening-devices.md) — Use `dinrail::Device` to open YARP devices and access their interfaces through interop plugins.
* [Exposing dinrail devices as YARP devices](yarp-migration-wrapping-devices.md) — Wrap native dinrail devices with YARP interface adapters and register them as YARP devices.

