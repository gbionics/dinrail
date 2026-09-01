# dr_battery_fake

`dr_battery_fake` is an in-memory battery device intended for tests and simulations. It implements
both `dinrail::IBattery` and `dinrail::IBatterySimulation`.

The device accepts no configuration parameters beyond the required `device` parameter. Opening it
resets its values to:

| Value | Default |
|:------|:--------|
| Voltage | `0.0` |
| Current | `0.0` |
| Charge | `100.0` |
| Status | `BatteryStatus::OkStandby` |
| Temperature | `25.0` |
| Information | `"Fake battery"` |

Use `IBatterySimulation` to update the values returned by `IBattery`. Charge values must be in the
inclusive range `[0, 100]`.
