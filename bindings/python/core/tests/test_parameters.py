# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

import datetime

import pytest

import dinrail


def test_parameters_values():
    parameters = dinrail.Parameters()
    parameters.put("flag", True)
    parameters.put("count", 3)
    parameters.put("gain", 0.5)
    parameters.put("name", "dinrail")
    parameters.put("period", datetime.timedelta(milliseconds=2))
    parameters.put("flags", [True, False])
    parameters.put("indices", [1, 2, 3])
    parameters.put("gains", (1, 2.5))
    parameters.put("names", ["a", "b"])
    parameters.put("periods", [datetime.timedelta(seconds=1)])
    parameters.put("empty", [])

    assert parameters.find("flag") is True
    assert parameters.find("count") == 3
    assert isinstance(parameters.find("count"), int)
    assert parameters.find("gain") == 0.5
    assert parameters.find("name") == "dinrail"
    assert parameters.find("period") == datetime.timedelta(milliseconds=2)
    assert parameters.find("flags") == [True, False]
    assert parameters.find("indices") == [1, 2, 3]
    assert parameters.find("gains") == [1.0, 2.5]
    assert parameters.find("names") == ["a", "b"]
    assert parameters.find("periods") == [datetime.timedelta(seconds=1)]
    assert parameters.find("empty") == []
    assert parameters.find("missing") is None

    assert parameters.check("count")
    assert not parameters.check("missing")
    assert "count" in parameters.getValueKeys()

    parameters.clear()
    assert parameters.getValueKeys() == []


def test_parameters_unsupported_values():
    parameters = dinrail.Parameters()
    with pytest.raises(TypeError):
        parameters.put("object", object())
    with pytest.raises(TypeError):
        parameters.put("mixed", [1, "a"])
    with pytest.raises(TypeError):
        dinrail.Parameters({1: "a"})


def test_parameters_groups():
    parameters = dinrail.Parameters({"device": "fake", "group": {"value": 1}})
    assert parameters.find("device") == "fake"
    assert parameters.getGroupKeys() == ["group"]
    assert parameters.findGroup("group").find("value") == 1
    assert parameters.findGroup("missing") is None

    added = parameters.addGroup("added")
    added.put("value", 2)
    assert parameters.findGroup("added").find("value") == 2


def test_status():
    assert dinrail.Status().ok()
    assert dinrail.Status().code() == dinrail.StatusCode.Ok

    status = dinrail.Status(dinrail.StatusCode.OutOfRange)
    assert not status.ok()
    assert status.code() == dinrail.StatusCode.OutOfRange
    assert str(status) == "OutOfRange"
    assert status == dinrail.Status(dinrail.StatusCode.OutOfRange)
    assert status != dinrail.Status()


def test_runtime_context():
    context = dinrail.RuntimeContext.getDefault()
    for device in context.listNativeDevices():
        assert device.name
        assert device.location