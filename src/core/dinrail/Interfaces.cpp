// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/IAxisInfo.h>
#include <dinrail/IBattery.h>
#include <dinrail/IDevice.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/IInterfaceView.h>
#include <dinrail/IInteropPlugin.h>
#include <dinrail/IJointFault.h>
#include <dinrail/IJoypadControl.h>
#include <dinrail/IMotor.h>
#include <dinrail/IMotorEncoders.h>
#include <dinrail/IPreciselyTimed.h>

namespace dinrail
{

IDevice::~IDevice() = default;

IInteropPlugin::~IInteropPlugin() = default;

IInterfaceView::~IInterfaceView() = default;

IAxisInfo::~IAxisInfo() = default;

IBattery::~IBattery() = default;

IBatterySimulation::~IBatterySimulation() = default;

IEncoders::~IEncoders() = default;

IEncodersSimulation::~IEncodersSimulation() = default;

IJointFault::~IJointFault() = default;

IJointFaultSimulation::~IJointFaultSimulation() = default;

IMotor::~IMotor() = default;

IMotorSimulation::~IMotorSimulation() = default;

IMotorEncoders::~IMotorEncoders() = default;

IMotorEncodersSimulation::~IMotorEncodersSimulation() = default;

IImpedanceAllSetPointsControl::~IImpedanceAllSetPointsControl() = default;

IJoypadControl::~IJoypadControl() = default;

IPreciselyTimed::~IPreciselyTimed() = default;

IPreciselyTimedSimulation::~IPreciselyTimedSimulation() = default;

} // namespace dinrail
