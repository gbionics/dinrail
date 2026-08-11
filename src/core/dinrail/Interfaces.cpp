// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/IAxisInfo.h>
#include <dinrail/IDevice.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/IInterfaceView.h>
#include <dinrail/IInteropPlugin.h>

namespace dinrail
{

IDevice::~IDevice() = default;

IInteropPlugin::~IInteropPlugin() = default;

IInterfaceView::~IInterfaceView() = default;

IAxisInfo::~IAxisInfo() = default;

IImpedanceAllSetPointsControl::~IImpedanceAllSetPointsControl() = default;

} // namespace dinrail
