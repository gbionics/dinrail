// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "DrCameraFake.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>
#include <opencv2/imgproc.hpp>
namespace dinrail
{
namespace
{
bool validImage(const cv::Mat& image, const cv::Mat& expected)
{
    return image.dims == 2 && !image.empty() && image.type() == expected.type()
           && image.size() == expected.size();
}
bool finite(double v)
{
    return std::isfinite(v);
}
// A compact television-style chart, drawn once and shared by all fake cameras.
cv::Mat makeTestPattern()
{
    cv::Mat chart(480, 640, CV_8UC3, cv::Scalar(160, 160, 160));
    for (int x = 0; x < chart.cols; x += 40)
        cv::line(chart, {x, 0}, {x, 479}, cv::Scalar(80, 80, 80));
    for (int y = 0; y < chart.rows; y += 40)
        cv::line(chart, {0, y}, {639, y}, cv::Scalar(80, 80, 80));

    cv::Mat circle(480, 640, CV_8UC3, cv::Scalar(235, 235, 235));
    // Scalars are in RGB order, matching dinrail's image contract.
    const cv::Scalar bars[] = {{255, 255, 255},
                               {255, 255, 0},
                               {0, 255, 255},
                               {0, 255, 0},
                               {255, 0, 255},
                               {255, 0, 0},
                               {0, 0, 255}};
    for (int i = 0; i < 7; ++i)
        cv::rectangle(circle, cv::Rect(131 + i * 54, 95, 54, 170), bars[i], cv::FILLED);
    for (int i = 0; i < 8; ++i)
        cv::rectangle(circle,
                      cv::Rect(128 + i * 48, 280, 48, 50),
                      cv::Scalar::all(i * 255 / 7),
                      cv::FILLED);
    for (int x = 120; x < 520; x += 4)
        cv::rectangle(circle, cv::Rect(x, 345, 2, 40), cv::Scalar::all(0), cv::FILLED);
    cv::putText(circle,
                "dr_camera_fake",
                {205, 80},
                cv::FONT_HERSHEY_SIMPLEX,
                0.6,
                cv::Scalar::all(0),
                1,
                cv::LINE_AA);
    cv::Mat mask = cv::Mat::zeros(480, 640, CV_8UC1);
    cv::circle(mask, {320, 220}, 190, cv::Scalar(255), cv::FILLED);
    circle.copyTo(chart, mask);
    cv::circle(chart, {320, 220}, 190, cv::Scalar::all(0), 2, cv::LINE_AA);
    cv::rectangle(chart, cv::Rect(40, 415, 560, 50), cv::Scalar::all(0), cv::FILLED);
    return chart;
}
Parameters intrinsic(int width, int height, double hfov, double vfov)
{
    Parameters p;
    p.put("physFocalLength", 0.0);
    const double pi = std::acos(-1.0);
    p.put("focalLengthX", width / (2 * std::tan(hfov * pi / 360)));
    p.put("focalLengthY", height / (2 * std::tan(vfov * pi / 360)));
    p.put("principalPointX", (width - 1) / 2.0);
    p.put("principalPointY", (height - 1) / 2.0);
    p.put("distortionModel", "cameraDistortionModelGroup");
    auto& model = p.addGroup("cameraDistortionModelGroup");
    model.put("name", "plumb_bob");
    for (auto key : {"k1", "k2", "k3", "t1", "t2"})
        model.put(key, 0.0);
    const std::vector<double> identity{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    p.put("rectificationMatrix", identity);
    return p;
}
} // namespace
Status DrCameraFake::ready(bool depth) const
{
    if (!m_opened)
        return Status{StatusCode::FailedPrecondition};
    if (depth && !m_rgbd)
        return Status{StatusCode::Unimplemented};
    return OkStatus();
}
Status DrCameraFake::featureReady(int feature) const
{
    if (auto status = ready(); !status.ok())
        return status;
    if (feature < 0 || feature >= static_cast<int>(m_features.size()))
        return Status{StatusCode::InvalidArgument};
    if (!m_features[feature].available)
        return Status{StatusCode::Unimplemented};
    return OkStatus();
}
bool DrCameraFake::open(const Parameters& config)
{
    std::lock_guard lock(m_mutex);
    m_opened = false;
    m_testPattern = false;
    if (config.check("test_pattern") && !config.getParameter("test_pattern", m_testPattern))
        return false;
    int width = 640, height = 480, depthWidth = 640, depthHeight = 480;
    std::string mode = "rgbd";
    for (auto entry : {std::pair{"width", &width},
                       {"height", &height},
                       {"depth_width", &depthWidth},
                       {"depth_height", &depthHeight}})
        if (config.check(entry.first) && !config.getParameter(entry.first, *entry.second))
            return false;
    if (config.check("mode") && !config.getParameter("mode", mode))
        return false;
    if ((mode != "rgbd" && mode != "framegrabber") || width <= 0 || height <= 0 || depthWidth <= 0
        || depthHeight <= 0)
        return false;
    m_rgbd = mode == "rgbd";
    try
    {
        m_raw = cv::Mat::zeros(height, width, CV_8UC1);
        m_rgb = cv::Mat::zeros(height, width, CV_8UC3);
        m_depth = cv::Mat::zeros(depthHeight, depthWidth, CV_32FC1);
        m_extrinsic = cv::Mat::eye(4, 4, CV_64FC1);
    } catch (const cv::Exception&)
    {
        return false;
    }
    m_rgbHFov = m_depthHFov = 60;
    m_rgbVFov = m_depthVFov = 45;
    m_rgbIntrinsic = intrinsic(width, height, 60, 45);
    m_depthIntrinsic = intrinsic(depthWidth, depthHeight, 60, 45);
    m_rgbStamp = m_depthStamp = m_lastStamp = m_errorStamp = {};
    m_rgbMirror = m_depthMirror = false;
    m_accuracy = 0.001;
    m_near = 0.1;
    m_far = 10;
    m_description = {BusType::Unknown, "Fake camera"};
    m_features.fill(Feature{});
    m_error.clear();
    m_status = RGBDSensorStatus::OkStandby;
    m_opened = true;
    return true;
}
bool DrCameraFake::close()
{
    std::lock_guard lock(m_mutex);
    m_opened = false;
    m_status = RGBDSensorStatus::NotReady;
    return true;
}
Stamp DrCameraFake::getLastInputStamp()
{
    std::lock_guard lock(m_mutex);
    return m_lastStamp;
}
void DrCameraFake::setLastInputStamp(const Stamp& stamp)
{
    std::lock_guard lock(m_mutex);
    m_lastStamp = stamp;
}

void DrCameraFake::updateTestPattern()
{
    if (!m_testPattern)
        return;

    static const cv::Mat pattern = makeTestPattern();
    // Resize only when the configured resolution changes. Repeated tiny
    // parallel jobs can keep OpenCV's worker pool busy between acquisitions.
    if (m_pattern.size() != m_rgb.size())
        cv::resize(pattern, m_pattern, m_rgb.size(), 0, 0, cv::INTER_AREA);
    m_rgb = m_pattern.clone();
    const double scale = std::min(m_rgb.cols / 640.0, m_rgb.rows / 480.0);
    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::floor<std::chrono::seconds>(now);
    const auto time = std::chrono::system_clock::to_time_t(seconds);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &time);
#else
    localtime_r(&time, &local);
#endif
    char text[32];
    std::strftime(text, sizeof(text), "%Y-%m-%d %H:%M:%S", &local);
    const int ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - seconds).count());
    cv::putText(m_rgb,
                cv::format("%s.%03d", text, ms),
                {75 * m_rgb.cols / 640, 448 * m_rgb.rows / 480},
                cv::FONT_HERSHEY_SIMPLEX,
                0.85 * scale,
                cv::Scalar::all(255),
                1,
                cv::LINE_AA);
    // One row retains the same packed pixels but avoids parallel_for over
    // image rows: TBB scheduling can cost far more than this small conversion.
    cv::Mat grayRow = m_raw.reshape(1, 1);
    cv::cvtColor(m_rgb.reshape(3, 1), grayRow, cv::COLOR_RGB2GRAY);
    m_rgbStamp.time = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
    m_rgbStamp.sequenceNumber = m_rgbStamp.sequenceNumber
                                        == std::numeric_limits<std::int32_t>::max()
                                    ? 0
                                    : m_rgbStamp.sequenceNumber + 1;
    m_lastStamp = m_rgbStamp;
}

Status DrCameraFake::getCameraDescription(CameraDescriptor& camera)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    camera = m_description;
    return OkStatus();
}

Status DrCameraFake::hasFeature(int feature, bool& hasFeature)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    if (feature < 0 || feature >= static_cast<int>(m_features.size()))
        return Status{StatusCode::InvalidArgument};
    hasFeature = m_features[feature].available;
    return OkStatus();
}

Status DrCameraFake::setFeature(int feature, double value)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    if (!finite(value) || value < 0 || value > 1)
        return Status{StatusCode::OutOfRange};
    f.value1 = value;
    return OkStatus();
}

Status DrCameraFake::getFeature(int feature, double& value)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    value = f.value1;
    return OkStatus();
}

Status DrCameraFake::setFeature(int feature, double value1, double value2)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    if (!finite(value1) || value1 < 0 || value1 > 1 || !finite(value2) || value2 < 0 || value2 > 1)
        return Status{StatusCode::OutOfRange};
    f.value1 = value1;
    f.value2 = value2;
    return OkStatus();
}

Status DrCameraFake::getFeature(int feature, double& value1, double& value2)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    value1 = f.value1;
    value2 = f.value2;
    return OkStatus();
}

Status DrCameraFake::hasOnOff(int feature, bool& hasOnOff)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    hasOnOff = f.onOff;
    return OkStatus();
}

Status DrCameraFake::setActive(int feature, bool onoff)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    if (!f.onOff)
        return Status{StatusCode::Unimplemented};
    f.active = onoff;
    return OkStatus();
}

Status DrCameraFake::getActive(int feature, bool& isActive)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    isActive = f.active;
    return OkStatus();
}

Status DrCameraFake::hasAuto(int feature, bool& hasAuto)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    hasAuto = f.automatic;
    return OkStatus();
}

Status DrCameraFake::hasManual(int feature, bool& hasManual)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    hasManual = f.manual;
    return OkStatus();
}

Status DrCameraFake::hasOnePush(int feature, bool& hasOnePush)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    hasOnePush = f.onePush;
    return OkStatus();
}

Status DrCameraFake::setMode(int feature, FeatureMode mode)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    if (mode != FeatureMode::Auto && mode != FeatureMode::Manual)
        return Status{StatusCode::InvalidArgument};
    if ((mode == FeatureMode::Auto && !f.automatic) || (mode == FeatureMode::Manual && !f.manual))
        return Status{StatusCode::Unimplemented};
    f.mode = mode;
    return OkStatus();
}

Status DrCameraFake::getMode(int feature, FeatureMode& mode)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    mode = f.mode;
    return OkStatus();
}

Status DrCameraFake::setOnePush(int feature)
{
    std::lock_guard lock(m_mutex);
    if (auto status = featureReady(feature); !status.ok())
        return status;
    auto& f = m_features[feature];
    return f.onePush ? OkStatus() : Status{StatusCode::Unimplemented};
}

int DrCameraFake::getRgbHeight()
{
    std::lock_guard lock(m_mutex);
    return m_opened ? m_rgb.rows : 0;
}

int DrCameraFake::getRgbWidth()
{
    std::lock_guard lock(m_mutex);
    return m_opened ? m_rgb.cols : 0;
}

Status DrCameraFake::getRgbSupportedConfigurations(std::vector<CameraConfig>& configurations)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    configurations = {{m_rgb.cols, m_rgb.rows, 30.0, PixelCoding::RGB}};
    return OkStatus();
}

Status DrCameraFake::getRgbResolution(int& width, int& height)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    width = m_rgb.cols;
    height = m_rgb.rows;
    return OkStatus();
}

Status DrCameraFake::setRgbResolution(int width, int height)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    if (width <= 0 || height <= 0)
        return Status{StatusCode::InvalidArgument};
    auto image = cv::Mat::zeros(height, width, m_rgb.type());
    auto raw = cv::Mat::zeros(height, width, CV_8UC1);
    m_raw = std::move(raw);
    m_rgb = std::move(image);
    m_rgbIntrinsic = intrinsic(width, height, m_rgbHFov, m_rgbVFov);
    return OkStatus();
}

Status DrCameraFake::getRgbFOV(double& horizontalFov, double& verticalFov)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    horizontalFov = m_rgbHFov;
    verticalFov = m_rgbVFov;
    return OkStatus();
}

Status DrCameraFake::setRgbFOV(double horizontalFov, double verticalFov)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    if (!finite(horizontalFov) || !finite(verticalFov) || horizontalFov <= 0 || horizontalFov >= 180
        || verticalFov <= 0 || verticalFov >= 180)
        return Status{StatusCode::InvalidArgument};
    m_rgbHFov = horizontalFov;
    m_rgbVFov = verticalFov;
    m_rgbIntrinsic = intrinsic(m_rgb.cols, m_rgb.rows, horizontalFov, verticalFov);
    return OkStatus();
}

Status DrCameraFake::getRgbIntrinsicParam(Parameters& intrinsic)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    intrinsic = m_rgbIntrinsic;
    return OkStatus();
}

Status DrCameraFake::getRgbMirroring(bool& mirror)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    mirror = m_rgbMirror;
    return OkStatus();
}

Status DrCameraFake::setRgbMirroring(bool mirror)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    m_rgbMirror = mirror;
    return OkStatus();
}

int DrCameraFake::getDepthHeight()
{
    std::lock_guard lock(m_mutex);
    return m_opened && m_rgbd ? m_depth.rows : 0;
}

int DrCameraFake::getDepthWidth()
{
    std::lock_guard lock(m_mutex);
    return m_opened && m_rgbd ? m_depth.cols : 0;
}

Status DrCameraFake::setDepthResolution(int width, int height)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (width <= 0 || height <= 0)
        return Status{StatusCode::InvalidArgument};
    auto image = cv::Mat::zeros(height, width, m_depth.type());
    m_depth = std::move(image);
    m_depthIntrinsic = intrinsic(width, height, m_depthHFov, m_depthVFov);
    return OkStatus();
}

Status DrCameraFake::getDepthFOV(double& horizontalFov, double& verticalFov)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    horizontalFov = m_depthHFov;
    verticalFov = m_depthVFov;
    return OkStatus();
}

Status DrCameraFake::setDepthFOV(double horizontalFov, double verticalFov)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (!finite(horizontalFov) || !finite(verticalFov) || horizontalFov <= 0 || horizontalFov >= 180
        || verticalFov <= 0 || verticalFov >= 180)
        return Status{StatusCode::InvalidArgument};
    m_depthHFov = horizontalFov;
    m_depthVFov = verticalFov;
    m_depthIntrinsic = intrinsic(m_depth.cols, m_depth.rows, horizontalFov, verticalFov);
    return OkStatus();
}

Status DrCameraFake::getDepthIntrinsicParam(Parameters& intrinsic)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    intrinsic = m_depthIntrinsic;
    return OkStatus();
}

double DrCameraFake::getDepthAccuracy()
{
    std::lock_guard lock(m_mutex);
    return m_opened && m_rgbd ? m_accuracy : 0;
}

Status DrCameraFake::setDepthAccuracy(double accuracy)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (!finite(accuracy) || accuracy <= 0)
        return Status{StatusCode::InvalidArgument};
    m_accuracy = accuracy;
    return OkStatus();
}

Status DrCameraFake::getDepthClipPlanes(double& nearPlane, double& farPlane)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    nearPlane = m_near;
    farPlane = m_far;
    return OkStatus();
}

Status DrCameraFake::setDepthClipPlanes(double nearPlane, double farPlane)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (!finite(nearPlane) || !finite(farPlane) || nearPlane < 0 || farPlane <= nearPlane)
        return Status{StatusCode::InvalidArgument};
    m_near = nearPlane;
    m_far = farPlane;
    return OkStatus();
}

Status DrCameraFake::getDepthMirroring(bool& mirror)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    mirror = m_depthMirror;
    return OkStatus();
}

Status DrCameraFake::setDepthMirroring(bool mirror)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    m_depthMirror = mirror;
    return OkStatus();
}

Status DrCameraFake::getExtrinsicParam(cv::Mat& extrinsic)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    extrinsic = m_extrinsic.clone();
    return OkStatus();
}

std::string DrCameraFake::getLastErrorMsg(Stamp* timeStamp)
{
    std::lock_guard lock(m_mutex);
    if (timeStamp)
        *timeStamp = m_errorStamp;
    return m_error;
}

Status DrCameraFake::getRgbImage(cv::Mat& rgbImage, Stamp* timeStamp)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    updateTestPattern();
    if (m_rgbMirror)
        cv::flip(m_rgb, rgbImage, 1);
    else
        rgbImage = m_rgb.clone();
    if (timeStamp)
        *timeStamp = m_rgbStamp;
    return OkStatus();
}

Status DrCameraFake::getDepthImage(cv::Mat& depthImage, Stamp* timeStamp)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (m_depthMirror)
        cv::flip(m_depth, depthImage, 1);
    else
        depthImage = m_depth.clone();
    if (timeStamp)
        *timeStamp = m_depthStamp;
    return OkStatus();
}

Status DrCameraFake::getImages(cv::Mat& colorFrame,
                               cv::Mat& depthFrame,
                               Stamp* colorStamp,
                               Stamp* depthStamp)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    updateTestPattern();
    if (m_rgbMirror)
        cv::flip(m_rgb, colorFrame, 1);
    else
        colorFrame = m_rgb.clone();
    if (m_depthMirror)
        cv::flip(m_depth, depthFrame, 1);
    else
        depthFrame = m_depth.clone();
    if (colorStamp)
        *colorStamp = m_rgbStamp;
    if (depthStamp)
        *depthStamp = m_depthStamp;
    return OkStatus();
}

RGBDSensorStatus DrCameraFake::getSensorStatus()
{
    std::lock_guard lock(m_mutex);
    return m_status;
}

int DrCameraFake::height() const
{
    std::lock_guard lock(m_mutex);
    return m_opened ? m_raw.rows : 0;
}

int DrCameraFake::width() const
{
    std::lock_guard lock(m_mutex);
    return m_opened ? m_raw.cols : 0;
}

Status DrCameraFake::getImage(cv::Mat& image)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    updateTestPattern();
    if (m_rgbMirror)
        cv::flip(m_raw, image, 1);
    else
        image = m_raw.clone();
    return OkStatus();
}

Status DrCameraFake::setImage(const cv::Mat& image)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    if (!validImage(image, m_raw))
        return Status{StatusCode::InvalidArgument};
    m_raw = image.clone();
    return OkStatus();
}

Status DrCameraFake::setRgbImage(const cv::Mat& image, const Stamp& stamp)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    if (!validImage(image, m_rgb))
        return Status{StatusCode::InvalidArgument};
    m_rgb = image.clone();
    m_rgbStamp = m_lastStamp = stamp;
    return OkStatus();
}

Status DrCameraFake::setDepthImage(const cv::Mat& image, const Stamp& stamp)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (!validImage(image, m_depth))
        return Status{StatusCode::InvalidArgument};
    m_depth = image.clone();
    m_depthStamp = m_lastStamp = stamp;
    return OkStatus();
}

Status DrCameraFake::setImages(const cv::Mat& rgb,
                               const cv::Mat& depth,
                               const Stamp& rgbStamp,
                               const Stamp& depthStamp)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (!validImage(rgb, m_rgb) || !validImage(depth, m_depth))
        return Status{StatusCode::InvalidArgument};
    auto newRgb = rgb.clone();
    auto newDepth = depth.clone();
    m_rgb = std::move(newRgb);
    m_depth = std::move(newDepth);
    m_rgbStamp = rgbStamp;
    m_depthStamp = depthStamp;
    m_lastStamp = rgbStamp;
    return OkStatus();
}

Status DrCameraFake::setRgbIntrinsicParam(const Parameters& intrinsic)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    m_rgbIntrinsic = intrinsic;
    return OkStatus();
}

Status DrCameraFake::setDepthIntrinsicParam(const Parameters& intrinsic)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    m_depthIntrinsic = intrinsic;
    return OkStatus();
}

Status DrCameraFake::setExtrinsicParam(const cv::Mat& extrinsic)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (extrinsic.dims != 2 || extrinsic.rows != 4 || extrinsic.cols != 4
        || extrinsic.type() != CV_64FC1 || !cv::checkRange(extrinsic))
        return Status{StatusCode::InvalidArgument};
    m_extrinsic = extrinsic.clone();
    return OkStatus();
}

Status DrCameraFake::setSensorStatus(RGBDSensorStatus status)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    if (status < RGBDSensorStatus::NotReady || status > RGBDSensorStatus::Timeout)
        return Status{StatusCode::InvalidArgument};
    m_status = status;
    return OkStatus();
}

Status DrCameraFake::setLastErrorMsg(const std::string& message, const Stamp& stamp)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(true); !status.ok())
        return status;
    m_error = message;
    m_errorStamp = stamp;
    return OkStatus();
}

Status DrCameraFake::setCameraDescription(const CameraDescriptor& camera)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    m_description = camera;
    return OkStatus();
}

Status DrCameraFake::setFeatureCapabilities(
    int feature, bool available, bool onOff, bool automatic, bool manual, bool onePush)
{
    std::lock_guard lock(m_mutex);
    if (auto status = ready(); !status.ok())
        return status;
    if (feature < 0 || feature >= static_cast<int>(m_features.size()))
        return Status{StatusCode::InvalidArgument};
    auto& f = m_features[feature];
    f.available = available;
    f.onOff = onOff;
    f.automatic = automatic;
    f.manual = manual;
    f.onePush = onePush;
    return OkStatus();
}
} // namespace dinrail
