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
#include <dinrail/MultipleAnalogSensorsInterfaces.h>

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

IThreeAxisGyroscopes::~IThreeAxisGyroscopes() = default;
IThreeAxisGyroscopesSimulation::~IThreeAxisGyroscopesSimulation() = default;
IThreeAxisLinearAccelerometers::~IThreeAxisLinearAccelerometers() = default;
IThreeAxisLinearAccelerometersSimulation::~IThreeAxisLinearAccelerometersSimulation() = default;
IThreeAxisAngularAccelerometers::~IThreeAxisAngularAccelerometers() = default;
IThreeAxisAngularAccelerometersSimulation::~IThreeAxisAngularAccelerometersSimulation() = default;
IThreeAxisMagnetometers::~IThreeAxisMagnetometers() = default;
IThreeAxisMagnetometersSimulation::~IThreeAxisMagnetometersSimulation() = default;
IPositionSensors::~IPositionSensors() = default;
IPositionSensorsSimulation::~IPositionSensorsSimulation() = default;
ILinearVelocitySensors::~ILinearVelocitySensors() = default;
ILinearVelocitySensorsSimulation::~ILinearVelocitySensorsSimulation() = default;
IOrientationSensors::~IOrientationSensors() = default;
IOrientationSensorsSimulation::~IOrientationSensorsSimulation() = default;
ITemperatureSensors::~ITemperatureSensors() = default;
ITemperatureSensorsSimulation::~ITemperatureSensorsSimulation() = default;
ISixAxisForceTorqueSensors::~ISixAxisForceTorqueSensors() = default;
ISixAxisForceTorqueSensorsSimulation::~ISixAxisForceTorqueSensorsSimulation() = default;
IContactLoadCellArrays::~IContactLoadCellArrays() = default;
IContactLoadCellArraysSimulation::~IContactLoadCellArraysSimulation() = default;
IEncoderArrays::~IEncoderArrays() = default;
IEncoderArraysSimulation::~IEncoderArraysSimulation() = default;
ISkinPatches::~ISkinPatches() = default;
ISkinPatchesSimulation::~ISkinPatchesSimulation() = default;

} // namespace dinrail
