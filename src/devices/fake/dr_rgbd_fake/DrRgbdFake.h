// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DR_RGBD_FAKE_H
#define DINRAIL_DR_RGBD_FAKE_H

#include <dinrail/IDevice.h>
#include <dinrail/IRGBDSensor.h>

#include <chrono>
#include <mutex>
#include <string>

namespace dinrail
{

/**
 * @brief Fake RGBD sensor driven through IRGBDSensorSimulation.
 *
 * When opened, the device provides an RGB8 color image filled with 128 and a depth image filled
 * with the @c depth parameter, both with sequence number 0. The images, which must keep the size
 * set when opened, and the status can then be changed with IRGBDSensorSimulation.
 *
 * Parameters: @c width (default 640), @c height (default 480), @c focal_length [pixel] (default
 * 500.0), @c depth [m] (default 1.0), @c framerate [Hz] (default 0.0). The principal point is the
 * center of the image, the color and depth frames coincide and there is no distortion. If
 * @c framerate is positive, the images are streamed: their sequence number is incremented, and
 * their time updated, @c framerate times per second since the device was opened.
 */
class DrRgbdFake final : public IDevice, public IRGBDSensor, public IRGBDSensorSimulation
{
public:
    bool open(const Parameters& config) override;
    bool close() override;

    RGBDSensorStatus getSensorStatus() override;
    std::string getLastErrorMessage() override;
    Status getRgbIntrinsics(CameraIntrinsics& intrinsics) override;
    Status getDepthIntrinsics(CameraIntrinsics& intrinsics) override;
    Status getDepthToRgbExtrinsics(CameraExtrinsics& extrinsics) override;
    Status getDepthClipPlanes(double& nearPlane, double& farPlane) override;
    Status getRgbImage(Image& rgbImage) override;
    Status getDepthImage(DepthImage& depthImage) override;
    Status getImages(Image& rgbImage, DepthImage& depthImage) override;
    Status getImagesStamp(Stamp& stamp) override;

    Status setImages(const Image& rgbImage, const DepthImage& depthImage) override;
    Status setSensorStatus(RGBDSensorStatus status) override;

private:
    /** Must be called with m_mutex locked. */
    void updateStream();

    std::mutex m_mutex;
    double m_framerate{0.0};
    bool m_enableRgb{true};
    std::chrono::steady_clock::time_point m_openTime;
    bool m_opened{false};
    CameraIntrinsics m_intrinsics;
    RGBDSensorStatus m_status{RGBDSensorStatus::NotReady};
    Image m_rgb;
    DepthImage m_depth;
};

} // namespace dinrail

#endif // DINRAIL_DR_RGBD_FAKE_H
