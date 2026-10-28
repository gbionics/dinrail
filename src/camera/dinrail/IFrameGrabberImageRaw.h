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

#ifndef DINRAIL_IFRAMEGRABBERIMAGERAW_H
#define DINRAIL_IFRAMEGRABBERIMAGERAW_H
#include <dinrail/Status.h>
#include <opencv2/core.hpp>

namespace dinrail
{
/** @brief Monochrome frame acquisition using CV_8UC1. Output images own their storage; callers may
 * modify them. Simulation setters copy input data. */
class IFrameGrabberImageRaw
{
public:
    /** @brief Virtual destructor. */
    virtual ~IFrameGrabberImageRaw();
    /**
     * @brief Get the monochrome frame height.
     * @return Frame height in pixels.
     */
    virtual int height() const = 0;
    /**
     * @brief Get the monochrome frame width.
     * @return Frame width in pixels.
     */
    virtual int width() const = 0;
    /**
     * @brief Acquire a monochrome frame.
     *
     * Returned images own their storage and may be modified by the caller.
     *
     * @param[out] image Output CV_8UC1 monochrome frame.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status getImage(cv::Mat& image) = 0;
};
/**
 * @brief Supply monochrome frames to a simulated framegrabber.
 * Input matrices are copied and must match the configured dimensions and pixel type.
 */
class IFrameGrabberImageRawSimulation
{
public:
    /** @brief Virtual destructor. */
    virtual ~IFrameGrabberImageRawSimulation();
    /**
     * @brief Supply the monochrome frame reported by the simulated camera.
     *
     * The implementation copies the input data, including non-contiguous matrices.
     *
     * @param[in] image Input CV_8UC1 monochrome frame.
     * @return Outcome of the operation; unsupported operations return StatusCode::Unimplemented.
     */
    virtual Status setImage(const cv::Mat& image) = 0;
};
} // namespace dinrail
#endif
