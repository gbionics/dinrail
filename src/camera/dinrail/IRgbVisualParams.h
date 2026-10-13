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

#ifndef DINRAIL_IRGBVISUALPARAMS_H
#define DINRAIL_IRGBVISUALPARAMS_H
#include <dinrail/CameraTypes.h>
#include <dinrail/Parameters.h>
#include <dinrail/Status.h>
#include <vector>

namespace dinrail
{
/**
 * @brief Configure RGB camera geometry and retrieve intrinsic calibration.
 *
 * Intrinsic Parameters follow the YARP camera calibration schema:
 * - physFocalLength: physical focal length in meters.
 * - focalLengthX, focalLengthY: focal lengths in pixels.
 * - principalPointX, principalPointY: principal point coordinates in pixels.
 * - rectificationMatrix: a row-major vector containing a 4x4 double matrix.
 * - distortionModel: name of the group holding the distortion parameters.
 *
 * The referenced distortion group contains name (for example "plumb_bob"),
 * radial coefficients k1, k2, k3 and tangential coefficients t1, t2.
 * Field of view is expressed in degrees.
 */
class IRgbVisualParams
{
public:
    /** @brief Virtual destructor. */
    virtual ~IRgbVisualParams();
    /**
     * @brief Get the RGB frame height.
     * @return Frame height in pixels.
     */
    virtual int getRgbHeight() = 0;
    /**
     * @brief Get the RGB frame width.
     * @return Frame width in pixels.
     */
    virtual int getRgbWidth() = 0;
    /**
     * @brief Get the supported RGB resolutions, frame rates and pixel encodings.
     *
     * @param[out] configurations Supported RGB camera configurations.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getRgbSupportedConfigurations(std::vector<CameraConfig>& configurations) = 0;
    /**
     * @brief Get the current RGB frame resolution.
     *
     * @param[out] width Frame width in pixels.
     * @param[out] height Frame height in pixels.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getRgbResolution(int& width, int& height) = 0;
    /**
     * @brief Set the RGB frame resolution.
     *
     * @param[in] width Frame width in pixels.
     * @param[in] height Frame height in pixels.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setRgbResolution(int width, int height) = 0;
    /**
     * @brief Get the RGB camera field of view.
     *
     * @param[out] horizontalFov Horizontal field of view in degrees.
     * @param[out] verticalFov Vertical field of view in degrees.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getRgbFOV(double& horizontalFov, double& verticalFov) = 0;
    /**
     * @brief Set the RGB camera field of view.
     *
     * @param[in] horizontalFov Horizontal field of view in degrees.
     * @param[in] verticalFov Vertical field of view in degrees.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setRgbFOV(double horizontalFov, double verticalFov) = 0;
    /**
     * @brief Get the RGB camera intrinsic calibration.
     *
     * Focal lengths and principal point coordinates use pixel units;
     * physFocalLength uses meters. Distortion coefficients are dimensionless.
     *
     * @param[out] intrinsic Intrinsic calibration; see IRgbVisualParams for the parameter schema.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getRgbIntrinsicParam(Parameters& intrinsic) = 0;
    /**
     * @brief Get whether acquired RGB frames are mirrored horizontally.
     *
     * @param[out] mirror True for horizontal mirroring, false otherwise.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getRgbMirroring(bool& mirror) = 0;
    /**
     * @brief Enable or disable horizontal mirroring of acquired RGB frames.
     *
     * @param[in] mirror True for horizontal mirroring, false otherwise.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setRgbMirroring(bool mirror) = 0;
};
} // namespace dinrail
#endif
