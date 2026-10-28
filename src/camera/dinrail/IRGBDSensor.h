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

#ifndef DINRAIL_IRGBDSENSOR_H
#define DINRAIL_IRGBDSENSOR_H
#include <dinrail/IDepthVisualParams.h>
#include <dinrail/IPreciselyTimed.h>
#include <dinrail/IRgbVisualParams.h>
#include <opencv2/core.hpp>

namespace dinrail
{
/** @brief RGBD acquisition: RGB is CV_8UC3 in RGB order; depth is CV_32FC1 in meters. Outputs own
 * their storage. getImages acquires a synchronized pair. Extrinsics are a 4x4 CV_64FC1 transform of
 * the depth optical frame in the RGB frame. Simulation setters copy inputs and getImages must
 * return each frame with its acquisition stamp. */
class IRGBDSensor : public IRgbVisualParams, public IDepthVisualParams
{
public:
    /** @brief Virtual destructor. */
    virtual ~IRGBDSensor();
    /**
     * @brief Get the pose of the depth optical frame with respect to the RGB optical frame.
     *
     * @param[out] extrinsic 4x4 CV_64FC1 homogeneous transform of the depth optical frame in the
     * RGB optical frame.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getExtrinsicParam(cv::Mat& extrinsic) = 0;
    /**
     * @brief Get the most recent sensor error message and its timestamp.
     *
     * @param[out] timeStamp Optional output acquisition or error stamp; ignored when nullptr.
     * @return Error description, or an empty string if no error is recorded.
     */
    virtual std::string getLastErrorMsg(Stamp* timeStamp = nullptr) = 0;
    /**
     * @brief Acquire an RGB frame and optionally its acquisition timestamp.
     *
     * Returned images own their storage and may be modified by the caller.
     *
     * @param[out] rgbImage Output CV_8UC3 image with channels in RGB order.
     * @param[out] timeStamp Optional output acquisition or error stamp; ignored when nullptr.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getRgbImage(cv::Mat& rgbImage, Stamp* timeStamp = nullptr) = 0;
    /**
     * @brief Acquire a depth frame and optionally its acquisition timestamp.
     *
     * Returned images own their storage and may be modified by the caller.
     *
     * @param[out] depthImage Output CV_32FC1 depth image containing distances in meters.
     * @param[out] timeStamp Optional output acquisition or error stamp; ignored when nullptr.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getDepthImage(cv::Mat& depthImage, Stamp* timeStamp = nullptr) = 0;
    /**
     * @brief Acquire an RGB and depth pair in a single call.
     *
     * Returned images own their storage and may be modified by the caller.
     * The implementation provides the best synchronization supported by its
     * acquisition policy. Each stamp refers to its corresponding frame.
     *
     * @param[out] colorFrame Output CV_8UC3 image with channels in RGB order.
     * @param[out] depthFrame Output CV_32FC1 depth image containing distances in meters.
     * @param[out] colorStamp Optional output RGB acquisition stamp; ignored when nullptr.
     * @param[out] depthStamp Optional output depth acquisition stamp; ignored when nullptr.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getImages(cv::Mat& colorFrame,
                             cv::Mat& depthFrame,
                             Stamp* colorStamp = nullptr,
                             Stamp* depthStamp = nullptr) = 0;
    /**
     * @brief Get the operating status of the RGBD sensor.
     * @return Current sensor operating status.
     */
    virtual RGBDSensorStatus getSensorStatus() = 0;
};
/**
 * @brief Supply frames, stamps, calibration, status and errors to a simulated RGBD sensor.
 * Image setters copy their inputs. Use IRgbVisualParams and IDepthVisualParams
 * to configure resolution, field of view, mirroring, depth accuracy and clip planes.
 */
class IRGBDSensorSimulation
{
public:
    /** @brief Virtual destructor. */
    virtual ~IRGBDSensorSimulation();
    /**
     * @brief Supply the RGB frame and acquisition stamp reported by the simulated sensor.
     *
     * The implementation copies the input data, including non-contiguous matrices.
     *
     * @param[in] image Input CV_8UC3 frame with RGB channel order.
     * @param[in] stamp Frame acquisition or error stamp.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setRgbImage(const cv::Mat& image, const Stamp& stamp) = 0;
    /**
     * @brief Supply the depth frame and acquisition stamp reported by the simulated sensor.
     *
     * The implementation copies the input data, including non-contiguous matrices.
     *
     * @param[in] image Input CV_32FC1 depth frame containing distances in meters.
     * @param[in] stamp Frame acquisition or error stamp.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setDepthImage(const cv::Mat& image, const Stamp& stamp) = 0;
    /**
     * @brief Supply an RGB and depth pair and their acquisition stamps atomically.
     *
     * The implementation copies the input data, including non-contiguous matrices.
     *
     * @param[in] rgb Input CV_8UC3 image with channels in RGB order.
     * @param[in] depth Input CV_32FC1 depth image containing distances in meters.
     * @param[in] rgbStamp RGB acquisition stamp.
     * @param[in] depthStamp Depth acquisition stamp.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setImages(const cv::Mat& rgb,
                             const cv::Mat& depth,
                             const Stamp& rgbStamp,
                             const Stamp& depthStamp) = 0;
    /**
     * @brief Set the RGB intrinsic calibration reported by the simulated sensor.
     *
     * Focal lengths and principal point coordinates use pixel units;
     * physFocalLength uses meters. Distortion coefficients are dimensionless.
     *
     * @param[in] intrinsic Intrinsic calibration; see IRgbVisualParams for the parameter schema.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setRgbIntrinsicParam(const Parameters& intrinsic) = 0;
    /**
     * @brief Set the depth intrinsic calibration reported by the simulated sensor.
     *
     * Focal lengths and principal point coordinates use pixel units;
     * physFocalLength uses meters. Distortion coefficients are dimensionless.
     *
     * @param[in] intrinsic Intrinsic calibration; see IRgbVisualParams for the parameter schema.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setDepthIntrinsicParam(const Parameters& intrinsic) = 0;
    /**
     * @brief Set the optical frame transform reported by the simulated sensor.
     *
     * The implementation copies the input data, including non-contiguous matrices.
     *
     * @param[in] extrinsic 4x4 CV_64FC1 homogeneous transform of the depth optical frame in the RGB
     * optical frame.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setExtrinsicParam(const cv::Mat& extrinsic) = 0;
    /**
     * @brief Set the operating status reported by the simulated sensor.
     *
     * @param[in] status Simulated RGBD sensor operating status.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setSensorStatus(RGBDSensorStatus status) = 0;
    /**
     * @brief Set the error message and timestamp reported by the simulated sensor.
     *
     * @param[in] message Error description to report; an empty string clears the error message.
     * @param[in] stamp Frame acquisition or error stamp.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setLastErrorMsg(const std::string& message, const Stamp& stamp) = 0;
};
} // namespace dinrail
#endif
