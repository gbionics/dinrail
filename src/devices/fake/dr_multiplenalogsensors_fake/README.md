# dr_multiplenalogsensors_fake Parameters

This document describes the parameters accepted by the
`dr_multiplenalogsensors_fake` dinrail device. The device exposes the multiple
analog sensor interfaces and is driven through their simulation interfaces.
Names and frame names are static metadata: configure them when opening the
device rather than through a simulation interface.

## Common Parameters

| Group name | Parameter name | Type | Units | Default Value | Required | Description | Notes |
|:----------:|:---------------|:----:|:-----:|:-------------:|:--------:|:------------|:------|
| - | number_of_sensors | int | - | 1 | 0 | Default number of sensors for every sensor family | Must be greater than or equal to zero. A family-specific count overrides it. |

## Per-family Parameters

Every sensor family accepts a `number_of_<family>s` parameter. It overrides
`number_of_sensors` for that family and must be greater than or equal to zero.
The singular `<family>_names` parameter is an optional vector of names; when
specified, it must contain exactly one entry for every sensor in that family.
Families that expose a frame-name accessor also accept
`<family>_frame_names`, with the same size requirement.

| Family | Count parameter | Name parameter | Frame-name parameter | Measurement |
|:-------|:----------------|:---------------|:---------------------|:------------|
| Three-axis gyroscope | number_of_three_axis_gyroscopes | three_axis_gyroscope_names | three_axis_gyroscope_frame_names | `[x, y, z]` angular velocity, degrees/s |
| Three-axis linear accelerometer | number_of_three_axis_linear_accelerometers | three_axis_linear_accelerometer_names | three_axis_linear_accelerometer_frame_names | `[x, y, z]` linear acceleration, m/s² |
| Three-axis angular accelerometer | number_of_three_axis_angular_accelerometers | three_axis_angular_accelerometer_names | three_axis_angular_accelerometer_frame_names | `[x, y, z]` angular acceleration, degrees/s² |
| Three-axis magnetometer | number_of_three_axis_magnetometers | three_axis_magnetometer_names | three_axis_magnetometer_frame_names | `[x, y, z]` magnetic field, tesla |
| Position sensor | number_of_position_sensors | position_sensor_names | position_sensor_frame_names | `[x, y, z]` position, m |
| Linear-velocity sensor | number_of_linear_velocity_sensors | linear_velocity_sensor_names | linear_velocity_sensor_frame_names | `[x, y, z]` linear velocity, m/s |
| Orientation sensor | number_of_orientation_sensors | orientation_sensor_names | orientation_sensor_frame_names | `[roll, pitch, yaw]`, degrees |
| Temperature sensor | number_of_temperature_sensors | temperature_sensor_names | temperature_sensor_frame_names | Temperature, °C |
| Six-axis force/torque sensor | number_of_six_axis_force_torque_sensors | six_axis_force_torque_sensor_names | six_axis_force_torque_sensor_frame_names | `[Fx, Fy, Fz, Tx, Ty, Tz]`, N and Nm |
| Contact load-cell array | number_of_contact_load_cell_arrays | contact_load_cell_array_names | - | Normal force values, N |
| Encoder array | number_of_encoder_arrays | encoder_array_names | - | Device-specific units |
| Skin patch | number_of_skin_patches | skin_patch_names | - | Implementation-specific units |

When names are omitted, the device generates names from the family prefix and
zero-based index (for example, `gyroscope0`). Frame names default to the sensor
name followed by `_frame`.

All measurement timestamps are expressed in seconds. Orientation values use
roll-pitch-yaw angles `[r, p, y]` with
`{}^f R_s = R_z(y) R_y(p) R_x(r)`, where `f` is the laboratory or
surface-fixed frame and `s` is the sensor frame. Roll and yaw range from -180°
to 180°; pitch ranges from -90° to 90°.

## Example Configuration (C++)

The project currently configures devices programmatically through
`dinrail::Parameters`.

```cpp
#include <dinrail/Parameters.h>

dinrail::Parameters options;
options.put("device", "dr_multiplenalogsensors_fake");
options.put("number_of_sensors", 2);
options.put("three_axis_gyroscope_names",
            std::vector<std::string>{"head_imu", "torso_imu"});
options.put("three_axis_gyroscope_frame_names",
            std::vector<std::string>{"head_imu_frame", "torso_imu_frame"});
options.put("number_of_temperature_sensors", 1);
options.put("temperature_sensor_names", std::vector<std::string>{"cpu_temperature"});
```
