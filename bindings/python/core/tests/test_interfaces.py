# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

import datetime

import pytest

import dinrail


def is_native_device_available(name):
    devices = dinrail.RuntimeContext.getDefault().listNativeDevices()
    return any(device.name == name for device in devices)


requires_controlboard = pytest.mark.skipif(
    not is_native_device_available("dr_controlboard_fake"),
    reason="dr_controlboard_fake plugin not available",
)

requires_sensors = pytest.mark.skipif(
    not is_native_device_available("dr_multiplenalogsensors_fake"),
    reason="dr_multiplenalogsensors_fake plugin not available",
)


def open_device(config):
    device = dinrail.Device()
    assert device.open({**config, "dinrail_device_type": "dinrail"})
    return device


@pytest.fixture
def controlboard():
    device = open_device(
        {
            "device": "dr_controlboard_fake",
            "number_of_joints": 3,
            "joint_names": ["shoulder", "elbow", "wrist"],
            "joint_type": ["revolute", "prismatic", "revolute"],
        }
    )
    yield device
    device.close()


@pytest.fixture
def sensors():
    device = open_device(
        {
            "device": "dr_multiplenalogsensors_fake",
            "number_of_sensors": 2,
            "three_axis_gyroscope_names": ["head_imu", "torso_imu"],
        }
    )
    yield device
    device.close()


@requires_controlboard
def test_axis_info(controlboard):
    axis_info = controlboard.view(dinrail.IAxisInfo)

    assert axis_info.getAxes() == (3, True)
    assert axis_info.getAxisName(1) == ("elbow", True)
    assert axis_info.getJointType(1) == (dinrail.JointType.PRISMATIC, True)
    # The outputs of a failed call are None
    assert axis_info.getAxisName(10) == (None, False)


@requires_controlboard
def test_encoders(controlboard):
    encoders = controlboard.view(dinrail.IEncoders)
    simulation = controlboard.view(dinrail.IEncodersSimulation)

    assert encoders.getAxes() == (3, True)

    assert simulation.setEncoders([1.0, 2.0, 3.0])
    assert encoders.getEncoders() == ([1.0, 2.0, 3.0], True)
    assert simulation.setEncoder(1, 5.0)
    assert encoders.getEncoder(1) == (5.0, True)
    assert encoders.getEncoder(10) == (None, False)

    assert simulation.setEncodersTimed([1.0, 2.0, 3.0], [0.1, 0.2, 0.3])
    assert encoders.getEncodersTimed() == ([1.0, 2.0, 3.0], [0.1, 0.2, 0.3], True)
    assert simulation.setEncoderTimed(0, 4.0, 0.5)
    assert encoders.getEncoderTimed(0) == (4.0, 0.5, True)

    assert simulation.setEncoderSpeeds([1.0, 1.5, 2.0])
    assert encoders.getEncoderSpeeds() == ([1.0, 1.5, 2.0], True)
    assert simulation.setEncoderSpeed(2, 3.0)
    assert encoders.getEncoderSpeed(2) == (3.0, True)

    assert simulation.setEncoderAccelerations([4.0, 5.0, 6.0])
    assert encoders.getEncoderAccelerations() == ([4.0, 5.0, 6.0], True)
    assert simulation.setEncoderAcceleration(0, 7.0)
    assert encoders.getEncoderAcceleration(0) == (7.0, True)

    # The vector size is checked by the device.
    assert not simulation.setEncoders([1.0])


@requires_controlboard
def test_motor(controlboard):
    motor = controlboard.view(dinrail.IMotor)
    simulation = controlboard.view(dinrail.IMotorSimulation)

    assert motor.getNumberOfMotors() == (3, True)

    assert simulation.setTemperatures([20.0, 30.0, 40.0])
    assert motor.getTemperatures() == ([20.0, 30.0, 40.0], True)
    assert simulation.setTemperature(1, 35.0)
    assert motor.getTemperature(1) == (35.0, True)

    assert motor.getTemperatureLimit(0) == (100.0, True)
    assert motor.setTemperatureLimit(0, 80.0)
    assert motor.getTemperatureLimit(0) == (80.0, True)

    assert motor.setGearboxRatio(2, 50.0)
    assert motor.getGearboxRatio(2) == (50.0, True)


@requires_controlboard
def test_motor_encoders(controlboard):
    encoders = controlboard.view(dinrail.IMotorEncoders)
    simulation = controlboard.view(dinrail.IMotorEncodersSimulation)

    assert encoders.getNumberOfMotorEncoders() == (3, True)
    assert encoders.getMotorEncoderCountsPerRevolution(0)[1] is True

    assert simulation.setMotorEncoders([1.0, 2.0, 3.0])
    assert encoders.getMotorEncoders() == ([1.0, 2.0, 3.0], True)
    assert simulation.setMotorEncoder(1, 9.0)
    assert encoders.getMotorEncoder(1) == (9.0, True)

    assert simulation.setMotorEncodersTimed([1.0, 2.0, 3.0], [0.1, 0.2, 0.3])
    assert encoders.getMotorEncodersTimed() == ([1.0, 2.0, 3.0], [0.1, 0.2, 0.3], True)
    assert simulation.setMotorEncoderTimed(0, 4.0, 0.5)
    assert encoders.getMotorEncoderTimed(0) == (4.0, 0.5, True)

    assert simulation.setMotorEncoderSpeeds([1.0, 2.0, 3.0])
    assert encoders.getMotorEncoderSpeeds() == ([1.0, 2.0, 3.0], True)
    assert simulation.setMotorEncoderSpeed(0, 8.0)
    assert encoders.getMotorEncoderSpeed(0) == (8.0, True)

    assert simulation.setMotorEncoderAccelerations([1.0, 2.0, 3.0])
    assert encoders.getMotorEncoderAccelerations() == ([1.0, 2.0, 3.0], True)
    assert simulation.setMotorEncoderAcceleration(0, 8.0)
    assert encoders.getMotorEncoderAcceleration(0) == (8.0, True)


@requires_controlboard
def test_joint_fault(controlboard):
    fault = controlboard.view(dinrail.IJointFault)
    simulation = controlboard.view(dinrail.IJointFaultSimulation)

    assert simulation.setLastJointFault(1, 5, "overheated")
    assert fault.getLastJointFault(1) == (5, "overheated", True)
    assert fault.getLastJointFault(0) == (0, "", True)
    assert fault.getLastJointFault(10) == (None, None, False)


@requires_controlboard
def test_impedance_all_set_points(controlboard):
    control = controlboard.view(dinrail.IImpedanceAllSetPointsControl)

    assert control.setSetPoint(0, 1.0, 2.0, 3.0, 4.0, 5.0)
    assert control.getSetPoint(0) == (1.0, 2.0, 3.0, 4.0, 5.0, True)

    assert control.setSetPoints([1, 2], [1.0, 2.0], [0.1, 0.2], [3.0, 4.0], [5.0, 6.0], [7.0, 8.0])
    assert control.getSetPoints([2, 1]) == (
        [2.0, 1.0],
        [0.2, 0.1],
        [4.0, 3.0],
        [6.0, 5.0],
        [8.0, 7.0],
        True,
    )

    assert control.setSetPoints([1.0] * 3, [2.0] * 3, [3.0] * 3, [4.0] * 3, [5.0] * 3)
    assert control.getSetPoints(3) == (
        [1.0] * 3,
        [2.0] * 3,
        [3.0] * 3,
        [4.0] * 3,
        [5.0] * 3,
        True,
    )

    assert control.getSetPoints([7]) == (None, None, None, None, None, False)
    assert control.getSetPoints(5) == (None, None, None, None, None, False)


@requires_controlboard
def test_precisely_timed(controlboard):
    timed = controlboard.view(dinrail.IPreciselyTimed)
    simulation = controlboard.view(dinrail.IPreciselyTimedSimulation)

    simulation.setLastInputStamp(dinrail.Stamp(datetime.timedelta(seconds=2), 7))

    stamp = timed.getLastInputStamp()
    assert stamp.time == datetime.timedelta(seconds=2)
    assert stamp.sequenceNumber == 7


@requires_sensors
def test_three_axis_vector_sensors(sensors):
    gyroscopes = sensors.view(dinrail.IThreeAxisGyroscopes)
    simulation = sensors.view(dinrail.IThreeAxisGyroscopesSimulation)

    assert gyroscopes.getNrOfThreeAxisGyroscopes() == 2
    assert gyroscopes.getThreeAxisGyroscopeName(1) == ("torso_imu", True)
    assert gyroscopes.getThreeAxisGyroscopeFrameName(0)[1] is True
    assert gyroscopes.getThreeAxisGyroscopeStatus(0) == dinrail.MAS_status.MAS_OK

    assert simulation.setThreeAxisGyroscopeMeasure(0, [1.0, 2.0, 3.0], 1.5)
    assert gyroscopes.getThreeAxisGyroscopeMeasure(0) == ([1.0, 2.0, 3.0], 1.5, True)
    assert simulation.setThreeAxisGyroscopeStatus(0, dinrail.MAS_status.MAS_ERROR)
    assert gyroscopes.getThreeAxisGyroscopeStatus(0) == dinrail.MAS_status.MAS_ERROR

    assert gyroscopes.getThreeAxisGyroscopeMeasure(5) == (None, None, False)


@requires_sensors
@pytest.mark.parametrize(
    "interface, simulation_interface, suffix, size",
    [
        ("IThreeAxisLinearAccelerometers", "IThreeAxisLinearAccelerometersSimulation", "ThreeAxisLinearAccelerometer", 3),
        ("IThreeAxisAngularAccelerometers", "IThreeAxisAngularAccelerometersSimulation", "ThreeAxisAngularAccelerometer", 3),
        ("IThreeAxisMagnetometers", "IThreeAxisMagnetometersSimulation", "ThreeAxisMagnetometer", 3),
        ("IPositionSensors", "IPositionSensorsSimulation", "PositionSensor", 3),
        ("ILinearVelocitySensors", "ILinearVelocitySensorsSimulation", "LinearVelocitySensor", 3),
        ("ISixAxisForceTorqueSensors", "ISixAxisForceTorqueSensorsSimulation", "SixAxisForceTorqueSensor", 6),
        ("IContactLoadCellArrays", "IContactLoadCellArraysSimulation", "ContactLoadCellArray", 4),
        ("IEncoderArrays", "IEncoderArraysSimulation", "EncoderArray", 4),
        ("ISkinPatches", "ISkinPatchesSimulation", "SkinPatch", 4),
    ],
)
def test_sensors_with_vector_measure(sensors, interface, simulation_interface, suffix, size):
    sensor = sensors.view(getattr(dinrail, interface))
    simulation = sensors.view(getattr(dinrail, simulation_interface))

    plural_suffix = {"SkinPatch": "SkinPatches"}.get(suffix, suffix + "s")
    assert getattr(sensor, f"getNrOf{plural_suffix}")() == 2
    assert getattr(sensor, f"get{suffix}Name")(0)[1] is True
    assert getattr(sensor, f"get{suffix}Status")(0) == dinrail.MAS_status.MAS_OK

    measure = [float(i) for i in range(size)]
    assert getattr(simulation, f"set{suffix}Measure")(1, measure, 2.5)
    assert getattr(sensor, f"get{suffix}Measure")(1) == (measure, 2.5, True)
    assert getattr(simulation, f"set{suffix}Status")(1, dinrail.MAS_status.MAS_OVF)
    assert getattr(sensor, f"get{suffix}Status")(1) == dinrail.MAS_status.MAS_OVF

    if hasattr(sensor, f"get{suffix}Size"):
        assert getattr(sensor, f"get{suffix}Size")(0) == size


@requires_sensors
def test_orientation_sensors(sensors):
    orientations = sensors.view(dinrail.IOrientationSensors)
    simulation = sensors.view(dinrail.IOrientationSensorsSimulation)

    assert orientations.getNrOfOrientationSensors() == 2
    assert orientations.getOrientationSensorName(0)[1] is True
    assert orientations.getOrientationSensorFrameName(0)[1] is True
    assert orientations.getOrientationSensorStatus(0) == dinrail.MAS_status.MAS_OK

    assert simulation.setOrientationSensorMeasureAsRollPitchYaw(0, [10.0, 20.0, 30.0], 1.0)
    assert orientations.getOrientationSensorMeasureAsRollPitchYaw(0) == (
        [10.0, 20.0, 30.0],
        1.0,
        True,
    )
    assert simulation.setOrientationSensorStatus(0, dinrail.MAS_status.MAS_TIMEOUT)
    assert orientations.getOrientationSensorStatus(0) == dinrail.MAS_status.MAS_TIMEOUT


@requires_sensors
def test_temperature_sensors(sensors):
    temperatures = sensors.view(dinrail.ITemperatureSensors)
    simulation = sensors.view(dinrail.ITemperatureSensorsSimulation)

    assert temperatures.getNrOfTemperatureSensors() == 2
    assert temperatures.getTemperatureSensorName(0)[1] is True
    assert temperatures.getTemperatureSensorFrameName(0)[1] is True
    assert temperatures.getTemperatureSensorStatus(0) == dinrail.MAS_status.MAS_OK

    assert simulation.setTemperatureSensorMeasure(0, 36.5, 2.0)
    assert temperatures.getTemperatureSensorMeasure(0) == (36.5, 2.0, True)
    assert temperatures.getTemperatureSensorMeasureAsVector(0) == ([36.5], 2.0, True)
    assert simulation.setTemperatureSensorStatus(0, dinrail.MAS_status.MAS_UNKNOWN)
    assert temperatures.getTemperatureSensorStatus(0) == dinrail.MAS_status.MAS_UNKNOWN


def test_joypad_interface_is_bound():
    # No joypad device is available in dinrail, so only the bindings are checked.
    for method in [
        "getAxisCount",
        "getButtonCount",
        "getHatCount",
        "getAxis",
        "getButton",
        "getHat",
        "reconnect",
        "getLastEvent",
    ]:
        assert hasattr(dinrail.IJoypadControl, method)
    assert dinrail.JoypadDeviceEvent.Connected != dinrail.JoypadDeviceEvent.NoEvent
