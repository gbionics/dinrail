// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/IFrameGrabberControls.h>
#include <dinrail/IFrameGrabberImageRaw.h>
#include <dinrail/IRGBDSensor.h>

namespace dinrail
{
IFrameGrabberControls::~IFrameGrabberControls() = default;
IFrameGrabberControlsSimulation::~IFrameGrabberControlsSimulation() = default;
IFrameGrabberImageRaw::~IFrameGrabberImageRaw() = default;
IFrameGrabberImageRawSimulation::~IFrameGrabberImageRawSimulation() = default;
IRgbVisualParams::~IRgbVisualParams() = default;
IDepthVisualParams::~IDepthVisualParams() = default;
IRGBDSensor::~IRGBDSensor() = default;
IRGBDSensorSimulation::~IRGBDSensorSimulation() = default;
} // namespace dinrail
