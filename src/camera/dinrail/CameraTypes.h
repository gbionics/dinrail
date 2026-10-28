// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: 2006-2010 RobotCub Consortium
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

/*
 * Camera API adapted from YARP, distributed under the BSD-3-Clause license.
 *
 * Copyright (c) 2006-2021 Istituto Italiano di Tecnologia (IIT).
 * Copyright (c) 2006-2010 RobotCub Consortium. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DINRAIL_CAMERATYPES_H
#define DINRAIL_CAMERATYPES_H
#include <string>

namespace dinrail
{
/** @brief Hardware bus used by a camera. */
enum class BusType
{
    Unknown = 0,
    FireWire,
    USB
};
/** @brief Camera feature adjustment mode. */
enum class FeatureMode
{
    Unknown = 0,
    Manual,
    Auto
};
/** @brief Camera hardware description. */
struct CameraDescriptor
{
    /** @brief Hardware bus type. */
    BusType busType{BusType::Unknown};
    /** @brief Human-readable hardware description. */
    std::string deviceDescription;
};
/** @brief Feature identifiers follow the camera feature ordering used by YARP. */
enum class CameraFeature : int
{
    Invalid = -1,
    Brightness = 0,
    Exposure,
    Sharpness,
    WhiteBalance,
    Hue,
    Saturation,
    Gamma,
    Shutter,
    Gain,
    Iris,
    Focus,
    Temperature,
    Trigger,
    TriggerDelay,
    WhiteShading,
    FrameRate,
    Zoom,
    Pan,
    Tilt,
    OpticalFilter,
    CaptureSize,
    CaptureQuality,
    Mirror,
    Count
};
/** @brief RGB matrices have RGB channel order; BGR matrices have OpenCV's usual BGR order. */
enum class PixelCoding
{
    RGB,
    BGR,
    Mono,
    DepthFloat
};
/** @brief Supported camera resolution, frame rate and pixel encoding. */
struct CameraConfig
{
    /** @brief Image width in pixels. */
    int width{0};
    /** @brief Image height in pixels. */
    int height{0};
    /** @brief Frame rate in frames per second. */
    double framerate{0};
    /** @brief Image pixel encoding and channel order. */
    PixelCoding pixelCoding{PixelCoding::RGB};
};
/** @brief Operating and error state of an RGBD sensor. */
enum class RGBDSensorStatus
{
    NotReady = 0,
    OkStandby,
    OkInUse,
    RgbError,
    DepthError,
    GenericError,
    Timeout
};
} // namespace dinrail
#endif
