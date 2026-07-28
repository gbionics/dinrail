# dr_controlboard_nws_yarp Parameters

This document describes the parameters currently accepted by the `dr_controlboard_nws_yarp` dinrail device.

This device is a control board network wrapper server for YARP: it exposes an attached control board device over the YARP network, opening the ports that a client (such as `dr_controlboard_nwc_yarp`) connects to.

## Parameters

| Group name | Parameter name                           | Type   | Units | Default Value | Required | Description                                                                      | Notes                                                                              |
|:----------:|:----------------------------------------:|:------:|:-----:|:-------------:|:--------:|:--------------------------------------------------------------------------------:|:------------------:|
| -          | name                                     | string | -     | -             | 1        | Prefix name of the ports opened by the device (must start with '/')              | Opened ports: `<name><namesuffix>/rpc:i`, `/command:i`, `/state:o`, `/stateExt:o` |
| -          | period                                   | double | s     | 0.02          | 0        | Period of the main thread                                                        | -                                                                                  |
| -          | namesuffix                               | string | -     | /dinrail      | 0        | Suffix appended to the base port prefix                                          | default keeps DinRail ports separated from stock YARP ports                       |
| -          | emulate_impedance_all_setpoints_control | bool   | -     | false         | 0        | Emulate dinrail IImpedanceAllSetPointsControl if subdevice does not expose it   | uses existing YARP interfaces, disabled by default                                |
