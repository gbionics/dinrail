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

## Blocking or non-blocking?

The interfaces define the semantics of each method. The tables below describe how `dr_controlboard_nwc_yarp` implements those methods when communicating with `dr_controlboard_nws_yarp`.

* **🔁 Blocking/RPC**: sends a request over the RPC port and waits for the server's reply.
* **📡 Non-blocking/Streaming**: reads the latest locally buffered state received on `/stateExt:i`, or queues a command on `/command:o` without waiting for an RPC reply.

Each row covers **all overloads** of the named method implemented by this client, including single-joint, joint-group, and all-joint variants where available.

### `yarp::dev::IAxisInfo`

| Method | Mode |
|:-------|:-----|
| `getAxes` | 🔁 Blocking/RPC |
| `getAxisName` | 🔁 Blocking/RPC |
| `getJointType` | 🔁 Blocking/RPC |

### `yarp::dev::IPidControl`

| Method | Mode |
|:-------|:-----|
| `setPid` | 🔁 Blocking/RPC |
| `setPids` | 🔁 Blocking/RPC |
| `setPidReference` | 🔁 Blocking/RPC |
| `setPidReferences` | 🔁 Blocking/RPC |
| `setPidErrorLimit` | 🔁 Blocking/RPC |
| `setPidErrorLimits` | 🔁 Blocking/RPC |
| `getPidError` | 🔁 Blocking/RPC |
| `getPidErrors` | 🔁 Blocking/RPC |
| `getPid` | 🔁 Blocking/RPC |
| `getPids` | 🔁 Blocking/RPC |
| `getPidReference` | 🔁 Blocking/RPC |
| `getPidReferences` | 🔁 Blocking/RPC |
| `getPidErrorLimit` | 🔁 Blocking/RPC |
| `getPidErrorLimits` | 🔁 Blocking/RPC |
| `resetPid` | 🔁 Blocking/RPC |
| `disablePid` | 🔁 Blocking/RPC |
| `enablePid` | 🔁 Blocking/RPC |
| `isPidEnabled` | 🔁 Blocking/RPC |
| `getPidOutput` | 🔁 Blocking/RPC |
| `getPidOutputs` | 🔁 Blocking/RPC |
| `setPidOffset` | 🔁 Blocking/RPC |

### `yarp::dev::IEncoders`

| Method | Mode |
|:-------|:-----|
| `getAxes` | 🔁 Blocking/RPC |
| `resetEncoder` | 🔁 Blocking/RPC |
| `resetEncoders` | 🔁 Blocking/RPC |
| `setEncoder` | 🔁 Blocking/RPC |
| `setEncoders` | 🔁 Blocking/RPC |
| `getEncoder` | 📡 Non-blocking/Streaming |
| `getEncoders` | 📡 Non-blocking/Streaming |
| `getEncoderSpeed` | 📡 Non-blocking/Streaming |
| `getEncoderSpeeds` | 📡 Non-blocking/Streaming |
| `getEncoderAcceleration` | 📡 Non-blocking/Streaming |
| `getEncoderAccelerations` | 📡 Non-blocking/Streaming |

### `yarp::dev::IEncodersTimed`

This interface also inherits all the methods listed under `IEncoders`.

| Method | Mode |
|:-------|:-----|
| `getEncoderTimed` | 📡 Non-blocking/Streaming |
| `getEncodersTimed` | 📡 Non-blocking/Streaming |

### `yarp::dev::IRemoteVariables`

| Method | Mode |
|:-------|:-----|
| `getRemoteVariable` | 🔁 Blocking/RPC |
| `setRemoteVariable` | 🔁 Blocking/RPC |
| `getRemoteVariablesList` | 🔁 Blocking/RPC |

### `yarp::dev::IMotor`

| Method | Mode |
|:-------|:-----|
| `getNumberOfMotors` | 🔁 Blocking/RPC |
| `getTemperature` | 📡 Non-blocking/Streaming |
| `getTemperatures` | 📡 Non-blocking/Streaming |
| `getTemperatureLimit` | 🔁 Blocking/RPC |
| `setTemperatureLimit` | 🔁 Blocking/RPC |
| `getGearboxRatio` | 🔁 Blocking/RPC |
| `setGearboxRatio` | 🔁 Blocking/RPC |

### `yarp::dev::IMotorEncoders`

| Method | Mode |
|:-------|:-----|
| `resetMotorEncoder` | 🔁 Blocking/RPC |
| `resetMotorEncoders` | 🔁 Blocking/RPC |
| `setMotorEncoder` | 🔁 Blocking/RPC |
| `setMotorEncoderCountsPerRevolution` | 🔁 Blocking/RPC |
| `getMotorEncoderCountsPerRevolution` | 🔁 Blocking/RPC |
| `setMotorEncoders` | 🔁 Blocking/RPC |
| `getMotorEncoder` | 📡 Non-blocking/Streaming |
| `getMotorEncoderTimed` | 📡 Non-blocking/Streaming |
| `getMotorEncoders` | 📡 Non-blocking/Streaming |
| `getMotorEncodersTimed` | 📡 Non-blocking/Streaming |
| `getMotorEncoderSpeed` | 📡 Non-blocking/Streaming |
| `getMotorEncoderSpeeds` | 📡 Non-blocking/Streaming |
| `getMotorEncoderAcceleration` | 📡 Non-blocking/Streaming |
| `getMotorEncoderAccelerations` | 📡 Non-blocking/Streaming |
| `getNumberOfMotorEncoders` | 🔁 Blocking/RPC |

### `yarp::dev::IPreciselyTimed`

| Method | Mode |
|:-------|:-----|
| `getLastInputStamp` | Non-blocking/`getLastInputStamp` returns the cached stamp last updated by a state getter or an RPC that extracts a timestamp. |

### `yarp::dev::IPositionControl`

| Method | Mode |
|:-------|:-----|
| `getAxes` | 🔁 Blocking/RPC |
| `positionMove` | 🔁 Blocking/RPC |
| `getTargetPosition` | 🔁 Blocking/RPC |
| `getTargetPositions` | 🔁 Blocking/RPC |
| `relativeMove` | 🔁 Blocking/RPC |
| `checkMotionDone` | 🔁 Blocking/RPC |
| `setRefSpeed` | 🔁 Blocking/RPC |
| `setRefSpeeds` | 🔁 Blocking/RPC |
| `setRefAcceleration` | 🔁 Blocking/RPC |
| `setRefAccelerations` | 🔁 Blocking/RPC |
| `getRefSpeed` | 🔁 Blocking/RPC |
| `getRefSpeeds` | 🔁 Blocking/RPC |
| `getRefAcceleration` | 🔁 Blocking/RPC |
| `getRefAccelerations` | 🔁 Blocking/RPC |
| `stop` | 🔁 Blocking/RPC |

### `yarp::dev::IJointFault`

| Method | Mode |
|:-------|:-----|
| `getLastJointFault` | 🔁 Blocking/RPC |

### `yarp::dev::IVelocityControl`

| Method | Mode |
|:-------|:-----|
| `getAxes` | 🔁 Blocking/RPC |
| `velocityMove` | 📡 Non-blocking/Streaming |
| `getRefVelocity` | 🔁 Blocking/RPC |
| `getRefVelocities` | 🔁 Blocking/RPC |
| `setRefAcceleration` | 🔁 Blocking/RPC |
| `setRefAccelerations` | 🔁 Blocking/RPC |
| `getRefAcceleration` | 🔁 Blocking/RPC |
| `getRefAccelerations` | 🔁 Blocking/RPC |
| `stop` | 🔁 Blocking/RPC |

### `yarp::dev::IAmplifierControl`

| Method | Mode |
|:-------|:-----|
| `enableAmp` | 🔁 Blocking/RPC |
| `disableAmp` | 🔁 Blocking/RPC |
| `getAmpStatus` | 🔁 Blocking/RPC |
| `setMaxCurrent` | 🔁 Blocking/RPC |
| `getMaxCurrent` | 🔁 Blocking/RPC |
| `getNominalCurrent` | 🔁 Blocking/RPC |
| `setNominalCurrent` | 🔁 Blocking/RPC |
| `getPeakCurrent` | 🔁 Blocking/RPC |
| `setPeakCurrent` | 🔁 Blocking/RPC |
| `getPWM` | 📡 Non-blocking/Streaming |
| `getPWMLimit` | 🔁 Blocking/RPC |
| `setPWMLimit` | 🔁 Blocking/RPC |
| `getPowerSupplyVoltage` | 🔁 Blocking/RPC |
| `getCurrent` | 📡 Non-blocking/Streaming |
| `getCurrents` | 📡 Non-blocking/Streaming |

### `yarp::dev::IControlLimits`

| Method | Mode |
|:-------|:-----|
| `setLimits` | 🔁 Blocking/RPC |
| `getLimits` | 🔁 Blocking/RPC |
| `setVelLimits` | 🔁 Blocking/RPC |
| `getVelLimits` | 🔁 Blocking/RPC |

### `yarp::dev::IControlCalibration`

| Method | Mode |
|:-------|:-----|
| `calibrateRobot` | 🔁 Blocking/RPC |
| `abortCalibration` | 🔁 Blocking/RPC |
| `abortPark` | 🔁 Blocking/RPC |
| `park` | 🔁 Blocking/RPC |
| `calibrateAxisWithParams` | 🔁 Blocking/RPC |
| `setCalibrationParameters` | 🔁 Blocking/RPC |
| `calibrationDone` | 🔁 Blocking/RPC |

`park(bool wait)` uses RPC regardless of `wait`; this implementation does not forward that argument.

### `yarp::dev::ITorqueControl`

| Method | Mode |
|:-------|:-----|
| `getAxes` | 🔁 Blocking/RPC |
| `getRefTorque` | 🔁 Blocking/RPC |
| `getRefTorques` | 🔁 Blocking/RPC |
| `setRefTorques` | 📡 Non-blocking/Streaming |
| `setRefTorque` | 📡 Non-blocking/Streaming |
| `setMotorTorqueParams` | 🔁 Blocking/RPC |
| `getMotorTorqueParams` | 🔁 Blocking/RPC |
| `getTorque` | 📡 Non-blocking/Streaming |
| `getTorques` | 📡 Non-blocking/Streaming |
| `getTorqueRange` | 🔁 Blocking/RPC |
| `getTorqueRanges` | 🔁 Blocking/RPC |

### `yarp::dev::IImpedanceControl`

| Method | Mode |
|:-------|:-----|
| `getImpedance` | 🔁 Blocking/RPC |
| `getImpedanceOffset` | 🔁 Blocking/RPC |
| `setImpedance` | 🔁 Blocking/RPC |
| `setImpedanceOffset` | 🔁 Blocking/RPC |
| `getCurrentImpedanceLimit` | 🔁 Blocking/RPC |

### `dinrail::IImpedanceAllSetPointsControl`

| Method | Mode |
|:-------|:-----|
| `setSetPoint` | 📡 Non-blocking/Streaming |
| `setSetPoints` | 📡 Non-blocking/Streaming |
| `getSetPoint` | 🔁 Blocking/RPC |
| `getSetPoints` | 🔁 Blocking/RPC |

### `yarp::dev::IControlMode`

| Method | Mode |
|:-------|:-----|
| `getControlMode` | 📡 Non-blocking/Streaming |
| `getControlModes` | 📡 Non-blocking/Streaming |
| `setControlMode` | 🔁 Blocking/RPC |
| `setControlModes` | 🔁 Blocking/RPC |

### `yarp::dev::IPositionDirect`

| Method | Mode |
|:-------|:-----|
| `getAxes` | 🔁 Blocking/RPC |
| `setPosition` | 📡 Non-blocking/Streaming |
| `setPositions` | 📡 Non-blocking/Streaming |
| `getRefPosition` | 🔁 Blocking/RPC |
| `getRefPositions` | 🔁 Blocking/RPC |

### `yarp::dev::IInteractionMode`

| Method | Mode |
|:-------|:-----|
| `getInteractionMode` | 📡 📡 Non-blocking/Streaming |
| `getInteractionModes` | 📡 📡 Non-blocking/Streaming |
| `setInteractionMode` | 🔁 🔁 Blocking/RPC |
| `setInteractionModes` | 🔁 🔁 Blocking/RPC |

### `yarp::dev::IRemoteCalibrator`

| Method | Mode |
|:-------|:-----|
| `isCalibratorDevicePresent` | 🔁 Blocking/RPC |
| `calibrateSingleJoint` | 🔁 Blocking/RPC |
| `calibrateWholePart` | 🔁 Blocking/RPC |
| `homingSingleJoint` | 🔁 Blocking/RPC |
| `homingWholePart` | 🔁 Blocking/RPC |
| `parkSingleJoint` | 🔁 Blocking/RPC |
| `parkWholePart` | 🔁 Blocking/RPC |
| `quitCalibrate` | 🔁 Blocking/RPC |
| `quitPark` | 🔁 Blocking/RPC |


`parkSingleJoint(int j, bool wait)` uses RPC regardless of `wait`; this implementation does not forward that argument.


### `yarp::dev::ICurrentControl`

| Method | Mode |
|:-------|:-----|
| `getRefCurrents` | 🔁 Blocking/RPC |
| `getRefCurrent` | 🔁 Blocking/RPC |
| `setRefCurrents` | 📡 Non-blocking/Streaming |
| `setRefCurrent` | 📡 Non-blocking/Streaming |
| `getCurrents` | 📡 Non-blocking/Streaming |
| `getCurrent` | 📡 Non-blocking/Streaming |
| `getCurrentRange` | 🔁 Blocking/RPC |
| `getCurrentRanges` | 🔁 Blocking/RPC |

### `yarp::dev::IPWMControl`

| Method | Mode |
|:-------|:-----|
| `setRefDutyCycle` | 📡 Non-blocking/Streaming |
| `setRefDutyCycles` | 📡 Non-blocking/Streaming |
| `getRefDutyCycle` | 🔁 Blocking/RPC |
| `getRefDutyCycles` | 🔁 Blocking/RPC |
| `getDutyCycle` | 📡 Non-blocking/Streaming |
| `getDutyCycles` | 📡 Non-blocking/Streaming |
