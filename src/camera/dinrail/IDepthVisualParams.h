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

#ifndef DINRAIL_IDEPTHVISUALPARAMS_H
#define DINRAIL_IDEPTHVISUALPARAMS_H
#include <dinrail/Parameters.h>
#include <dinrail/Status.h>

namespace dinrail
{
/** @brief Depth camera geometry. FOV is in degrees; accuracy and clip planes are in meters. */
class IDepthVisualParams
{
public:
    /** @brief Virtual destructor. */
    virtual ~IDepthVisualParams();
    /**
     * @brief Get the depth frame height.
     * @return Frame height in pixels.
     */
    virtual int getDepthHeight() = 0;
    /**
     * @brief Get the depth frame width.
     * @return Frame width in pixels.
     */
    virtual int getDepthWidth() = 0;
    /**
     * @brief Set the depth frame resolution.
     *
     * @param[in] width Frame width in pixels.
     * @param[in] height Frame height in pixels.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setDepthResolution(int width, int height) = 0;
    /**
     * @brief Get the depth camera field of view.
     *
     * @param[out] horizontalFov Horizontal field of view in degrees.
     * @param[out] verticalFov Vertical field of view in degrees.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getDepthFOV(double& horizontalFov, double& verticalFov) = 0;
    /**
     * @brief Set the depth camera field of view.
     *
     * @param[in] horizontalFov Horizontal field of view in degrees.
     * @param[in] verticalFov Vertical field of view in degrees.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setDepthFOV(double horizontalFov, double verticalFov) = 0;
    /**
     * @brief Get the depth camera intrinsic calibration.
     *
     * Focal lengths and principal point coordinates use pixel units;
     * physFocalLength uses meters. Distortion coefficients are dimensionless.
     *
     * @param[out] intrinsic Intrinsic calibration; see IRgbVisualParams for the parameter schema.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getDepthIntrinsicParam(Parameters& intrinsic) = 0;
    /**
     * @brief Get the smallest detectable variation in depth.
     * @return Smallest detectable depth variation in meters.
     */
    virtual double getDepthAccuracy() = 0;
    /**
     * @brief Set the smallest detectable variation in depth.
     *
     * @param[in] accuracy Smallest detectable depth variation in meters.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setDepthAccuracy(double accuracy) = 0;
    /**
     * @brief Get the distances delimiting the depth sensing range.
     *
     * @param[out] nearPlane Minimum measurable distance in meters.
     * @param[out] farPlane Maximum measurable distance in meters.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getDepthClipPlanes(double& nearPlane, double& farPlane) = 0;
    /**
     * @brief Set the distances delimiting the depth sensing range.
     *
     * @param[in] nearPlane Minimum measurable distance in meters.
     * @param[in] farPlane Maximum measurable distance in meters.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setDepthClipPlanes(double nearPlane, double farPlane) = 0;
    /**
     * @brief Get whether acquired depth frames are mirrored horizontally.
     *
     * @param[out] mirror True for horizontal mirroring, false otherwise.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getDepthMirroring(bool& mirror) = 0;
    /**
     * @brief Enable or disable horizontal mirroring of acquired depth frames.
     *
     * @param[in] mirror True for horizontal mirroring, false otherwise.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setDepthMirroring(bool mirror) = 0;
};
} // namespace dinrail
#endif
