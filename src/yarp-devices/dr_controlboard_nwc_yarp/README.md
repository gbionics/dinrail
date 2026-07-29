# dr_controlboard_nwc_yarp Parameters

This document describes the parameters currently accepted by the `dr_controlboard_nwc_yarp` YARP device.

This device is the client side of the control board: it connects to a remote control board using the YARP network, communicating through the ports opened by the `dr_controlboard_nws_yarp` device.
This makes it possible to use a device exposing control board methods from a different process (or even a different computer) than the one that opened the control board device.

## Parameters

| Group name | Parameter name      | Type   | Units | Default Value | Required | Description                                                        | Notes                                                                    |
|:----------:|:-------------------:|:------:|:-----:|:-------------:|:--------:|:------------------------------------------------------------------:|:------------------:|
| -          | remote              | string | -     | -             | 1        | Prefix of the port to which to connect.                            | -                                                                        |
| -          | local               | string | -     | -             | 1        | Port prefix of the port opened by this device.                     | -                                                                        |
| -          | namesuffix          | string | -     | /dinrail      | 0        | Suffix appended to local and remote prefixes.                      | default keeps DinRail ports separated from stock YARP ports used by `remotecontrolboard` and `controlBoard_nws_yarp`.             |
| -          | writeStrict         | string | -     | -             | 0        | It can be 'on' or 'off'                                            | See implementation                                                        |
| -          | carrier             | string | -     | udp           | 0        | carrier used for streaming robot state                             | -                                                                        |
| -          | timeout             | float  | -     | 0.5           | 0        | timeout for the input port which receives the streamed robot state | -                                                                        |
| local_qos  | enable              | bool   | -     | false         | 0        | Enable the usage of local Qos                                      | -                                                                        |
| local_qos  | thread_priority     | int    | -     | 0             | 0        | Local Qos. See https://yarp.it/latest/channelprioritization.html   | -                                                                        |
| local_qos  | thread_policy       | int    | -     | 0             | 0        | Local Qos. See https://yarp.it/latest/channelprioritization.html   | -                                                                        |
| local_qos  | packet_priority     | string | -     | -             | 0        | Local Qos. See https://yarp.it/latest/channelprioritization.html   | -                                                                        |
| remote_qos | enable              | bool   | -     | false         | 0        | Enable the usage of remote Qos                                     | -                                                                        |
| remote_qos | thread_priority     | int    | -     | 0             | 0        | Remote Qos. See https://yarp.it/latest/channelprioritization.html  | -                                                                        |
| remote_qos | thread_policy       | int    | -     | 0             | 0        | Remote Qos. See https://yarp.it/latest/channelprioritization.html. | -                                                                        |
| remote_qos | packet_priority     | string | -     | -             | 0        | Remote Qos. See https://yarp.it/latest/channelprioritization.html. | -                                                                        |
| -          | ignoreProtocolCheck | bool   | -     | false         | 0        | For development purpose only                                        | -                                                                        |
| -          | diagnostic          | bool   | -     | false         | 0        | For development purpose only                                        | -                                                                        |
