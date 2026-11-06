# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

import gc

import pytest

import dinrail


def is_native_device_available(name):
    devices = dinrail.RuntimeContext.getDefault().listNativeDevices()
    return any(device.name == name for device in devices)


def test_open_missing_device():
    device = dinrail.Device()
    assert not device.open(
        {"device": "dinrail_bindings_missing_device", "dinrail_device_type": "dinrail"}
    )
    assert not device.isValid()
    assert device.view(dinrail.IBattery) is None


def test_view_unregistered_interface():
    class NotAnInterface:
        pass

    device = dinrail.Device()
    with pytest.raises(TypeError):
        device.view(NotAnInterface)


@pytest.mark.skipif(
    not is_native_device_available("dr_battery_fake"),
    reason="dr_battery_fake plugin not available",
)
def test_battery_interfaces():
    device = dinrail.Device()
    assert device.open(
        dinrail.Parameters({"device": "dr_battery_fake", "dinrail_device_type": "dinrail"})
    )
    assert device.isValid()

    battery = device.view(dinrail.IBattery)
    simulation = device.view(dinrail.IBatterySimulation)
    assert isinstance(battery, dinrail.IBattery)
    assert isinstance(simulation, dinrail.IBatterySimulation)

    assert simulation.setBatteryVoltage(48.0).ok()
    assert simulation.setBatteryStatus(dinrail.BatteryStatus.OkInUse).ok()
    assert simulation.setBatteryInfo("Test battery").ok()
    assert simulation.setBatteryCharge(101.0).code() == dinrail.StatusCode.OutOfRange

    voltage, status = battery.getBatteryVoltage()
    assert status.ok()
    assert voltage == 48.0

    battery_status, status = battery.getBatteryStatus()
    assert status.ok()
    assert battery_status == dinrail.BatteryStatus.OkInUse

    info, status = battery.getBatteryInfo()
    assert status.ok()
    assert info == "Test battery"

    assert device.close()
    assert not device.isValid()


def test_interface_registered_by_external_bindings():
    # Both the interface and its bindings are unknown to dinrail.
    import dinrail_test_greeter

    device = dinrail.Device()
    assert device.open(
        {"device": "test_greeter", "dinrail_device_type": "dinrail", "greeting": "Ciao"}
    )
    assert device.view(dinrail.IBattery) is None

    greeter = device.view(dinrail_test_greeter.IGreeter)
    assert isinstance(greeter, dinrail_test_greeter.IGreeter)
    assert greeter.greet("dinrail") == "Ciao dinrail"

    # The view keeps the device alive.
    del device
    gc.collect()
    assert greeter.greet("again") == "Ciao again"
