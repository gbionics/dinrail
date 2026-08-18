// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/IAxisInfo.h>
#include <dinrail/IDevice.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/IJoypadControl.h>

namespace dinrail
{

IAxisInfo::~IAxisInfo() = default;

IDevice::~IDevice() = default;

IImpedanceAllSetPointsControl::~IImpedanceAllSetPointsControl() = default;

IJoypadControl::~IJoypadControl() = default;
} // namespace dinrail
