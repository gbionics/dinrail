// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/IAxisInfo.h>
#include <dinrail/IDevice.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/IInterfaceView.h>
#include <dinrail/IInteropPlugin.h>
#include <dinrail/IJoypadControl.h>
#include <dinrail/IPreciselyTimed.h>

namespace dinrail
{

IDevice::~IDevice() = default;

IInteropPlugin::~IInteropPlugin() = default;

IInterfaceView::~IInterfaceView() = default;

IAxisInfo::~IAxisInfo() = default;

IEncoders::~IEncoders() = default;

IEncodersSimulation::~IEncodersSimulation() = default;

IImpedanceAllSetPointsControl::~IImpedanceAllSetPointsControl() = default;

IJoypadControl::~IJoypadControl() = default;

IPreciselyTimed::~IPreciselyTimed() = default;

IPreciselyTimedSimulation::~IPreciselyTimedSimulation() = default;

} // namespace dinrail
