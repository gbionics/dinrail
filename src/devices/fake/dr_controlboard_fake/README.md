# dr_controlboard_fake Parameters

This document describes the parameters currently accepted by the `dr_controlboard_fake` dinrail device.

## Parameters

<<<<<<< HEAD:src/devices/fake/dr_controlboard_fake/README.md
- `device` (string)
  - Must be `"dr_controlboard_fake"`.

## Optional Parameters

- `number_of_joints` (int)
  - Number of joints exposed by the fake device.
  - Default: `1`.
  - Must be greater than `0`.

- `joint_names` (vector of string)
  - Custom names for each joint.
  - If provided, its size must be exactly `number_of_joints` (or the effective joint count).
  - Default names are generated as `joint_0`, `joint_1`, ..., `joint_N`.

- `joint_type` (vector of string)
  - Joint type for each joint.
  - Supported values (case-insensitive):
    - `"revolute"`
    - `"prismatic"`
  - If provided, its size must be exactly `number_of_joints` (or the effective joint count).
  - Default type for all joints: `revolute`.

- `GENERAL.Joints` (int)
  - Deprecated compatibility parameter for YARP's `dr_controlboard_fake`.
  - Supported only for compatibility.
  - Used only when `number_of_joints` is not provided.

## Parameter Precedence

1. `number_of_joints`
2. `GENERAL.Joints` (deprecated compatibility fallback)
3. default `1`
=======
| Group name | Parameter name  | Type              | Units | Default Value | Required | Description                                                       | Notes                                                              |
|:----------:|:---------------:|:----------------:|:-----:|:-------------:|:--------:|:-----------------------------------------------------------------:|:------------------:|
| -          | number_of_joints| int               | -     | 1             | 0        | Number of joints exposed by the fake device                       | Must be greater than 0                                             |
| -          | joint_names     | vector of string  | -     | auto-generated| 0        | Custom names for each joint                                       | Size must match number_of_joints; defaults to joint_0, joint_1, ... |
| -          | joint_type      | vector of string  | -     | revolute      | 0        | Joint type for each joint (revolute or prismatic)                 | Size must match number_of_joints; case-insensitive                 |
| GENERAL          | Joints  | int               | -     | -             | 0        | Deprecated compatibility parameter for YARP's fakeMotionControl   | Used only if number_of_joints is not provided                      |
>>>>>>> origin/main:src/devices/fake/fakeMotionControl/README.md

## Example Configuration (C++)

The project currently configures devices programmatically through `dinrail::Parameters`.

```cpp
#include <dinrail/Parameters.h>

dinrail::Parameters opts;
opts.put("device", "dr_controlboard_fake");
opts.put("number_of_joints", 3);
opts.put("joint_names", std::vector<std::string>{"shoulder", "elbow", "wrist"});
opts.put("joint_type", std::vector<std::string>{"revolute", "prismatic", "revolute"});
```
