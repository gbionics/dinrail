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

#ifndef DINRAIL_IFRAMEGRABBERCONTROLS_H
#define DINRAIL_IFRAMEGRABBERCONTROLS_H
#include <dinrail/CameraTypes.h>
#include <dinrail/Status.h>

namespace dinrail
{
/** @brief Camera controls. Feature values are normalized to [0,1]. Unsupported operations return
 * Unimplemented. */
class IFrameGrabberControls
{
public:
    /** @brief Virtual destructor. */
    virtual ~IFrameGrabberControls();
    /**
     * @brief Get a description of the camera hardware and its bus.
     *
     * @param[out] camera Camera bus type and hardware description.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getCameraDescription(CameraDescriptor& camera) = 0;
    /**
     * @brief Check whether the camera provides a feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] hasFeature True if the feature is available.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status hasFeature(int feature, bool& hasFeature) = 0;
    /**
     * @brief Set the normalized value of a camera feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[in] value Feature value normalized to [0, 1].
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setFeature(int feature, double value) = 0;
    /**
     * @brief Get the normalized value of a camera feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] value Feature value normalized to [0, 1].
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getFeature(int feature, double& value) = 0;
    /**
     * @brief Set the normalized value of a camera feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[in] value1 First feature component normalized to [0, 1], for example one white balance
     * channel.
     * @param[in] value2 Second feature component normalized to [0, 1], for example the other white
     * balance channel.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setFeature(int feature, double value1, double value2) = 0;
    /**
     * @brief Get the normalized value of a camera feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] value1 First feature component normalized to [0, 1], for example one white
     * balance channel.
     * @param[out] value2 Second feature component normalized to [0, 1], for example the other white
     * balance channel.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getFeature(int feature, double& value1, double& value2) = 0;
    /**
     * @brief Check whether a feature can be enabled and disabled.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] hasOnOff True if the feature can be enabled and disabled.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status hasOnOff(int feature, bool& hasOnOff) = 0;
    /**
     * @brief Enable or disable a camera feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[in] onoff True to enable the feature, false to disable it.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setActive(int feature, bool onoff) = 0;
    /**
     * @brief Get the enabled state of a camera feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] isActive True if the feature is enabled.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getActive(int feature, bool& isActive) = 0;
    /**
     * @brief Check whether a feature supports automatic adjustment.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] hasAuto True if automatic adjustment is supported.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status hasAuto(int feature, bool& hasAuto) = 0;
    /**
     * @brief Check whether a feature supports manual adjustment.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] hasManual True if manual adjustment is supported.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status hasManual(int feature, bool& hasManual) = 0;
    /**
     * @brief Check whether a feature supports a single automatic adjustment.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] hasOnePush True if a single automatic adjustment is supported.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status hasOnePush(int feature, bool& hasOnePush) = 0;
    /**
     * @brief Select manual or automatic adjustment for a feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[in] mode Manual or automatic feature adjustment mode.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setMode(int feature, FeatureMode mode) = 0;
    /**
     * @brief Get the current adjustment mode of a feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[out] mode Manual or automatic feature adjustment mode.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getMode(int feature, FeatureMode& mode) = 0;
    /**
     * @brief Request a single automatic adjustment of a feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setOnePush(int feature) = 0;
};
/**
 * @brief Supply the hardware description and feature capabilities of a simulated camera.
 * Use IFrameGrabberControls to set feature values, activation and adjustment modes.
 */
class IFrameGrabberControlsSimulation
{
public:
    /** @brief Virtual destructor. */
    virtual ~IFrameGrabberControlsSimulation();
    /**
     * @brief Set the hardware description reported by the simulated camera.
     *
     * @param[in] camera Camera bus type and hardware description.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setCameraDescription(const CameraDescriptor& camera) = 0;
    /**
     * @brief Set the capabilities reported for a simulated camera feature.
     *
     * @param[in] feature Feature identifier, using the numeric values of CameraFeature.
     * @param[in] available Whether the simulated feature is available.
     * @param[in] onOff Whether the simulated feature can be enabled and disabled.
     * @param[in] automatic Whether the simulated feature supports automatic adjustment.
     * @param[in] manual Whether the simulated feature supports manual adjustment.
     * @param[in] onePush Whether the simulated feature supports a single automatic adjustment.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setFeatureCapabilities(
        int feature, bool available, bool onOff, bool automatic, bool manual, bool onePush) = 0;
};
} // namespace dinrail
#endif
