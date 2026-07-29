# dr_controlboard_fake Parameters

This document describes the parameters currently accepted by the `dr_controlboard_fake` dinrail device.

## Parameters

| Group name | Parameter name  | Type              | Units | Default Value | Required | Description                                                       | Notes                                                              |
|:----------:|:---------------:|:----------------:|:-----:|:-------------:|:--------:|:-----------------------------------------------------------------:|:------------------:|
| -          | number_of_joints| int               | -     | 1             | 0        | Number of joints exposed by the fake device                       | Must be greater than 0                                             |
| -          | joint_names     | vector of string  | -     | auto-generated| 0        | Custom names for each joint                                       | Size must match number_of_joints; defaults to joint_0, joint_1, ... |
| -          | joint_type      | vector of string  | -     | revolute      | 0        | Joint type for each joint (revolute or prismatic)                 | Size must match number_of_joints; case-insensitive                 |
| GENERAL    | Joints          | int               | -     | -             | 0        | Deprecated compatibility parameter for YARP's fakeMotionControl   | Used only if number_of_joints is not provided                      |

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
