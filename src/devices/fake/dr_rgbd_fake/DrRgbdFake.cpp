// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "DrRgbdFake.h"

#include <limits>

namespace dinrail
{

bool DrRgbdFake::open(const Parameters& config)
{
    int width = 640;
    int height = 480;
    double focalLength = 500.0;
    double depth = 1.0;
    config.getParameter("width", width);
    config.getParameter("height", height);
    config.getParameter("focal_length", focalLength);
    config.getParameter("depth", depth);
    double framerate = 0.0;
    config.getParameter("framerate", framerate);
    bool enableRgb = true;
    config.getParameter("enable_rgb", enableRgb);
    if (width <= 0 || height <= 0 || focalLength <= 0.0 || framerate < 0.0)
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_intrinsics = CameraIntrinsics{};
    m_intrinsics.width = static_cast<std::uint32_t>(width);
    m_intrinsics.height = static_cast<std::uint32_t>(height);
    m_intrinsics.focalLengthX = focalLength;
    m_intrinsics.focalLengthY = focalLength;
    m_intrinsics.principalPointX = width / 2.0;
    m_intrinsics.principalPointY = height / 2.0;

    m_rgb = Image{};
    m_rgb.width = m_intrinsics.width;
    m_rgb.height = m_intrinsics.height;
    m_rgb.format = PixelFormat::Rgb8;
    m_rgb.data.assign(std::size_t{m_rgb.width} * m_rgb.height * 3, 128);
    m_depth = DepthImage{};
    m_depth.width = m_intrinsics.width;
    m_depth.height = m_intrinsics.height;
    m_depth.data.assign(std::size_t{m_depth.width} * m_depth.height, static_cast<float>(depth));
    m_status = RGBDSensorStatus::Ok;
    m_framerate = framerate;
    m_enableRgb = enableRgb;
    m_openTime = std::chrono::steady_clock::now();
    m_opened = true;
    return true;
}

void DrRgbdFake::updateStream()
{
    if (m_framerate <= 0.0)
    {
        return;
    }
    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - m_openTime;
    const auto frame = static_cast<std::int32_t>(elapsed.count() * m_framerate);
    if (frame == m_depth.stamp.sequenceNumber)
    {
        return;
    }
    const Stamp stamp{std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::duration<double>(frame / m_framerate)),
                      frame};
    m_rgb.stamp = stamp;
    m_depth.stamp = stamp;
}

bool DrRgbdFake::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_opened = false;
    m_status = RGBDSensorStatus::NotReady;
    return true;
}

RGBDSensorStatus DrRgbdFake::getSensorStatus()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_status;
}

std::string DrRgbdFake::getLastErrorMessage()
{
    return {};
}

Status DrRgbdFake::getRgbIntrinsics(CameraIntrinsics& intrinsics)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !m_enableRgb)
    {
        return Status(StatusCode::FailedPrecondition);
    }
    intrinsics = m_intrinsics;
    return Status();
}

Status DrRgbdFake::getDepthIntrinsics(CameraIntrinsics& intrinsics)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return Status(StatusCode::FailedPrecondition);
    }
    intrinsics = m_intrinsics;
    return Status();
}

Status DrRgbdFake::getDepthToRgbExtrinsics(CameraExtrinsics& extrinsics)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !m_enableRgb)
    {
        return Status(StatusCode::FailedPrecondition);
    }
    extrinsics = CameraExtrinsics{};
    return Status();
}

Status DrRgbdFake::getDepthClipPlanes(double& nearPlane, double& farPlane)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return Status(StatusCode::FailedPrecondition);
    }
    nearPlane = 0.0;
    farPlane = std::numeric_limits<double>::infinity();
    return Status();
}

Status DrRgbdFake::getRgbImage(Image& rgbImage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return Status(StatusCode::Unavailable);
    }
    if (!m_enableRgb)
    {
        return Status(StatusCode::FailedPrecondition);
    }
    updateStream();
    rgbImage = m_rgb;
    return Status();
}

Status DrRgbdFake::getDepthImage(DepthImage& depthImage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return Status(StatusCode::Unavailable);
    }
    updateStream();
    depthImage = m_depth;
    return Status();
}

Status DrRgbdFake::getImages(Image& rgbImage, DepthImage& depthImage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return Status(StatusCode::Unavailable);
    }
    if (!m_enableRgb)
    {
        return Status(StatusCode::FailedPrecondition);
    }
    updateStream();
    rgbImage = m_rgb;
    depthImage = m_depth;
    return Status();
}

Status DrRgbdFake::getImagesStamp(Stamp& stamp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return Status(StatusCode::Unavailable);
    }
    updateStream();
    stamp = m_depth.stamp;
    return Status();
}

Status DrRgbdFake::setImages(const Image& rgbImage, const DepthImage& depthImage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return Status(StatusCode::FailedPrecondition);
    }
    if (rgbImage.width != m_intrinsics.width || rgbImage.height != m_intrinsics.height
        || depthImage.width != m_intrinsics.width || depthImage.height != m_intrinsics.height
        || rgbImage.data.size()
               != std::size_t{rgbImage.width} * rgbImage.height * bytesPerPixel(rgbImage.format)
        || depthImage.data.size() != std::size_t{depthImage.width} * depthImage.height)
    {
        return Status(StatusCode::InvalidArgument);
    }
    m_rgb = rgbImage;
    m_depth = depthImage;
    return Status();
}

Status DrRgbdFake::setSensorStatus(RGBDSensorStatus status)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status = status;
    return Status();
}

} // namespace dinrail
