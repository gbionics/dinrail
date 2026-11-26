// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpRGBDSensorAdapters.h>

#include <dinrail/YarpStampConverter.h>

#include <yarp/sig/IntrinsicParams.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>

namespace dinrail
{

namespace
{

int toYarpPixelCode(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::Rgb8:
        return VOCAB_PIXEL_RGB;
    case PixelFormat::Bgr8:
        return VOCAB_PIXEL_BGR;
    case PixelFormat::Mono8:
        return VOCAB_PIXEL_MONO;
    case PixelFormat::Mono16:
        return VOCAB_PIXEL_MONO16;
    case PixelFormat::Unknown:
        break;
    }
    return VOCAB_PIXEL_INVALID;
}

PixelFormat toDinrailPixelFormat(int pixelCode)
{
    switch (pixelCode)
    {
    case VOCAB_PIXEL_RGB:
        return PixelFormat::Rgb8;
    case VOCAB_PIXEL_BGR:
        return PixelFormat::Bgr8;
    case VOCAB_PIXEL_MONO:
        return PixelFormat::Mono8;
    case VOCAB_PIXEL_MONO16:
        return PixelFormat::Mono16;
    default:
        return PixelFormat::Unknown;
    }
}

double radiansToDegrees(double angle)
{
    return angle * 180.0 / std::numbers::pi;
}

/** Map a value in [0, 1] onto the range of an option, rounded to its step. */
double fromNormalized(double normalized, const CameraOptionRange& range)
{
    double value = range.min + std::clamp(normalized, 0.0, 1.0) * (range.max - range.min);
    if (range.step > 0.0)
    {
        value = range.min + std::round((value - range.min) / range.step) * range.step;
    }
    return std::clamp(value, range.min, range.max);
}

double toNormalized(double value, const CameraOptionRange& range)
{
    if (range.max <= range.min)
    {
        return 0.0;
    }
    return std::clamp((value - range.min) / (range.max - range.min), 0.0, 1.0);
}

} // namespace

bool toYarpImage(const Image& image, yarp::sig::FlexImage& yarpImage)
{
    const int pixelCode = toYarpPixelCode(image.format);
    const std::size_t pixelSize = bytesPerPixel(image.format);
    if (pixelCode == VOCAB_PIXEL_INVALID
        || image.data.size() != std::size_t{image.width} * image.height * pixelSize)
    {
        return false;
    }

    yarpImage.setPixelCode(pixelCode);
    yarpImage.resize(image.width, image.height);
    const std::size_t rowBytes = std::size_t{image.width} * pixelSize;
    for (std::size_t row = 0; row < image.height; ++row)
    {
        std::memcpy(yarpImage.getRow(row), image.data.data() + row * rowBytes, rowBytes);
    }
    return true;
}

bool toDinrailImage(const yarp::sig::FlexImage& yarpImage,
                    const yarp::os::Stamp& stamp,
                    Image& image)
{
    const PixelFormat format = toDinrailPixelFormat(yarpImage.getPixelCode());
    if (format == PixelFormat::Unknown)
    {
        return false;
    }

    image.format = format;
    image.width = static_cast<std::uint32_t>(yarpImage.width());
    image.height = static_cast<std::uint32_t>(yarpImage.height());
    image.stamp = toDinrailStamp(stamp);
    const std::size_t rowBytes = std::size_t{image.width} * bytesPerPixel(format);
    image.data.resize(rowBytes * image.height);
    for (std::size_t row = 0; row < image.height; ++row)
    {
        std::memcpy(image.data.data() + row * rowBytes, yarpImage.getRow(row), rowBytes);
    }
    return true;
}

void toYarpImage(const DepthImage& image, yarp::sig::ImageOf<yarp::sig::PixelFloat>& yarpImage)
{
    yarpImage.resize(image.width, image.height);
    const std::size_t rowBytes = std::size_t{image.width} * sizeof(float);
    for (std::size_t row = 0; row < image.height; ++row)
    {
        std::memcpy(yarpImage.getRow(row), image.data.data() + row * image.width, rowBytes);
    }
}

void toDinrailImage(const yarp::sig::ImageOf<yarp::sig::PixelFloat>& yarpImage,
                    const yarp::os::Stamp& stamp,
                    DepthImage& image)
{
    image.width = static_cast<std::uint32_t>(yarpImage.width());
    image.height = static_cast<std::uint32_t>(yarpImage.height());
    image.stamp = toDinrailStamp(stamp);
    image.data.resize(std::size_t{image.width} * image.height);
    const std::size_t rowBytes = std::size_t{image.width} * sizeof(float);
    for (std::size_t row = 0; row < image.height; ++row)
    {
        std::memcpy(image.data.data() + row * image.width, yarpImage.getRow(row), rowBytes);
    }
}

void toYarpIntrinsics(const CameraIntrinsics& intrinsics, yarp::os::Property& yarpIntrinsics)
{
    yarp::sig::IntrinsicParams params;
    params.focalLengthX = intrinsics.focalLengthX;
    params.focalLengthY = intrinsics.focalLengthY;
    params.principalPointX = intrinsics.principalPointX;
    params.principalPointY = intrinsics.principalPointY;
    switch (intrinsics.distortionModel)
    {
    case DistortionModel::None:
        params.distortionModel.type = yarp::sig::YarpDistortion::YARP_DISTORTION_NONE;
        break;
    case DistortionModel::BrownConrady:
    case DistortionModel::ModifiedBrownConrady:
    case DistortionModel::InverseBrownConrady:
        params.distortionModel.type = yarp::sig::YarpDistortion::YARP_PLUMB_BOB;
        break;
    case DistortionModel::FTheta:
    case DistortionModel::KannalaBrandt4:
        params.distortionModel.type = yarp::sig::YarpDistortion::YARP_FISH_EYE;
        break;
    }
    params.distortionModel.k1 = intrinsics.distortionCoefficients[0];
    params.distortionModel.k2 = intrinsics.distortionCoefficients[1];
    params.distortionModel.t1 = intrinsics.distortionCoefficients[2];
    params.distortionModel.t2 = intrinsics.distortionCoefficients[3];
    params.distortionModel.k3 = intrinsics.distortionCoefficients[4];
    params.toProperty(yarpIntrinsics);
}

CameraIntrinsics
toDinrailIntrinsics(const yarp::os::Property& yarpIntrinsics, int width, int height)
{
    const yarp::sig::IntrinsicParams params(yarpIntrinsics, true);
    CameraIntrinsics intrinsics;
    intrinsics.width = static_cast<std::uint32_t>(std::max(width, 0));
    intrinsics.height = static_cast<std::uint32_t>(std::max(height, 0));
    intrinsics.focalLengthX = params.focalLengthX;
    intrinsics.focalLengthY = params.focalLengthY;
    intrinsics.principalPointX = params.principalPointX;
    intrinsics.principalPointY = params.principalPointY;
    switch (params.distortionModel.type)
    {
    case yarp::sig::YarpDistortion::YARP_PLUMB_BOB:
        intrinsics.distortionModel = DistortionModel::BrownConrady;
        break;
    case yarp::sig::YarpDistortion::YARP_FISH_EYE:
        intrinsics.distortionModel = DistortionModel::KannalaBrandt4;
        break;
    default:
        intrinsics.distortionModel = DistortionModel::None;
        break;
    }
    intrinsics.distortionCoefficients = {params.distortionModel.k1,
                                         params.distortionModel.k2,
                                         params.distortionModel.t1,
                                         params.distortionModel.t2,
                                         params.distortionModel.k3};
    return intrinsics;
}

void toYarpExtrinsics(const CameraExtrinsics& extrinsics, yarp::sig::Matrix& yarpExtrinsics)
{
    yarpExtrinsics.resize(4, 4);
    yarpExtrinsics.eye();
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            yarpExtrinsics(row, column) = extrinsics.rotation[row * 3 + column];
        }
        yarpExtrinsics(row, 3) = extrinsics.translation[row];
    }
}

bool toDinrailExtrinsics(const yarp::sig::Matrix& yarpExtrinsics, CameraExtrinsics& extrinsics)
{
    if (yarpExtrinsics.rows() != 4 || yarpExtrinsics.cols() != 4)
    {
        return false;
    }
    for (std::size_t row = 0; row < 3; ++row)
    {
        for (std::size_t column = 0; column < 3; ++column)
        {
            extrinsics.rotation[row * 3 + column] = yarpExtrinsics(row, column);
        }
        extrinsics.translation[row] = yarpExtrinsics(row, 3);
    }
    return true;
}

yarp::dev::IRGBDSensor::RGBDSensor_status toYarpSensorStatus(RGBDSensorStatus status)
{
    switch (status)
    {
    case RGBDSensorStatus::NotReady:
        return yarp::dev::IRGBDSensor::RGBD_SENSOR_NOT_READY;
    case RGBDSensorStatus::Ok:
        return yarp::dev::IRGBDSensor::RGBD_SENSOR_OK_IN_USE;
    case RGBDSensorStatus::Timeout:
        return yarp::dev::IRGBDSensor::RGBD_SENSOR_TIMEOUT;
    case RGBDSensorStatus::Disconnected:
    case RGBDSensorStatus::Error:
        break;
    }
    return yarp::dev::IRGBDSensor::RGBD_SENSOR_GENERIC_ERROR;
}

RGBDSensorStatus toDinrailSensorStatus(yarp::dev::IRGBDSensor::RGBDSensor_status status)
{
    switch (status)
    {
    case yarp::dev::IRGBDSensor::RGBD_SENSOR_NOT_READY:
        return RGBDSensorStatus::NotReady;
    case yarp::dev::IRGBDSensor::RGBD_SENSOR_OK_STANDBY:
    case yarp::dev::IRGBDSensor::RGBD_SENSOR_OK_IN_USE:
        return RGBDSensorStatus::Ok;
    case yarp::dev::IRGBDSensor::RGBD_SENSOR_TIMEOUT:
        return RGBDSensorStatus::Timeout;
    default:
        return RGBDSensorStatus::Error;
    }
}

void fieldOfView(const CameraIntrinsics& intrinsics, double& horizontalFov, double& verticalFov)
{
    horizontalFov = intrinsics.focalLengthX > 0.0
                        ? radiansToDegrees(
                              2.0 * std::atan(intrinsics.width / (2.0 * intrinsics.focalLengthX)))
                        : 0.0;
    verticalFov = intrinsics.focalLengthY > 0.0
                      ? radiansToDegrees(
                            2.0 * std::atan(intrinsics.height / (2.0 * intrinsics.focalLengthY)))
                      : 0.0;
}

// ---- yarp::dev::IRGBDSensor from dinrail::IRGBDSensor ----

using YarpRGBDFromDinrail = InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>;

int YarpRGBDFromDinrail::getRgbHeight()
{
    CameraIntrinsics intrinsics;
    return source().getRgbIntrinsics(intrinsics).ok() ? static_cast<int>(intrinsics.height) : 0;
}

int YarpRGBDFromDinrail::getRgbWidth()
{
    CameraIntrinsics intrinsics;
    return source().getRgbIntrinsics(intrinsics).ok() ? static_cast<int>(intrinsics.width) : 0;
}

bool YarpRGBDFromDinrail::getRgbResolution(int& width, int& height)
{
    CameraIntrinsics intrinsics;
    if (!source().getRgbIntrinsics(intrinsics).ok())
    {
        return false;
    }
    width = static_cast<int>(intrinsics.width);
    height = static_cast<int>(intrinsics.height);
    return true;
}

bool YarpRGBDFromDinrail::setRgbResolution(int, int)
{
    return false;
}

bool YarpRGBDFromDinrail::getRgbFOV(double& horizontalFov, double& verticalFov)
{
    CameraIntrinsics intrinsics;
    if (!source().getRgbIntrinsics(intrinsics).ok())
    {
        return false;
    }
    fieldOfView(intrinsics, horizontalFov, verticalFov);
    return true;
}

bool YarpRGBDFromDinrail::setRgbFOV(double, double)
{
    return false;
}

bool YarpRGBDFromDinrail::getRgbIntrinsicParam(yarp::os::Property& intrinsic)
{
    CameraIntrinsics intrinsics;
    if (!source().getRgbIntrinsics(intrinsics).ok())
    {
        return false;
    }
    toYarpIntrinsics(intrinsics, intrinsic);
    return true;
}

bool YarpRGBDFromDinrail::getRgbMirroring(bool& mirror)
{
    mirror = false;
    return true;
}

bool YarpRGBDFromDinrail::setRgbMirroring(bool mirror)
{
    return !mirror;
}

int YarpRGBDFromDinrail::getDepthHeight()
{
    CameraIntrinsics intrinsics;
    return source().getDepthIntrinsics(intrinsics).ok() ? static_cast<int>(intrinsics.height) : 0;
}

int YarpRGBDFromDinrail::getDepthWidth()
{
    CameraIntrinsics intrinsics;
    return source().getDepthIntrinsics(intrinsics).ok() ? static_cast<int>(intrinsics.width) : 0;
}

bool YarpRGBDFromDinrail::setDepthResolution(int, int)
{
    return false;
}

bool YarpRGBDFromDinrail::getDepthFOV(double& horizontalFov, double& verticalFov)
{
    CameraIntrinsics intrinsics;
    if (!source().getDepthIntrinsics(intrinsics).ok())
    {
        return false;
    }
    fieldOfView(intrinsics, horizontalFov, verticalFov);
    return true;
}

bool YarpRGBDFromDinrail::setDepthFOV(double, double)
{
    return false;
}

bool YarpRGBDFromDinrail::getDepthIntrinsicParam(yarp::os::Property& intrinsic)
{
    CameraIntrinsics intrinsics;
    if (!source().getDepthIntrinsics(intrinsics).ok())
    {
        return false;
    }
    toYarpIntrinsics(intrinsics, intrinsic);
    return true;
}

double YarpRGBDFromDinrail::getDepthAccuracy()
{
    // Depth images are already converted to meters, the resolution is not exposed.
    return 0.0;
}

bool YarpRGBDFromDinrail::setDepthAccuracy(double)
{
    return false;
}

bool YarpRGBDFromDinrail::getDepthClipPlanes(double& nearPlane, double& farPlane)
{
    return source().getDepthClipPlanes(nearPlane, farPlane).ok();
}

bool YarpRGBDFromDinrail::setDepthClipPlanes(double, double)
{
    return false;
}

bool YarpRGBDFromDinrail::getDepthMirroring(bool& mirror)
{
    mirror = false;
    return true;
}

bool YarpRGBDFromDinrail::setDepthMirroring(bool mirror)
{
    return !mirror;
}

bool YarpRGBDFromDinrail::getExtrinsicParam(yarp::sig::Matrix& extrinsic)
{
    CameraExtrinsics extrinsics;
    if (!source().getDepthToRgbExtrinsics(extrinsics).ok())
    {
        return false;
    }
    toYarpExtrinsics(extrinsics, extrinsic);
    return true;
}

std::string YarpRGBDFromDinrail::getLastErrorMsg(yarp::os::Stamp*)
{
    return source().getLastErrorMessage();
}

bool YarpRGBDFromDinrail::getRgbImage(yarp::sig::FlexImage& rgbImage, yarp::os::Stamp* timeStamp)
{
    if (!source().getRgbImage(m_rgb).ok() || !toYarpImage(m_rgb, rgbImage))
    {
        return false;
    }
    if (timeStamp != nullptr)
    {
        *timeStamp = toYarpStamp(m_rgb.stamp);
    }
    return true;
}

bool YarpRGBDFromDinrail::getDepthImage(yarp::sig::ImageOf<yarp::sig::PixelFloat>& depthImage,
                                        yarp::os::Stamp* timeStamp)
{
    if (!source().getDepthImage(m_depth).ok())
    {
        return false;
    }
    toYarpImage(m_depth, depthImage);
    if (timeStamp != nullptr)
    {
        *timeStamp = toYarpStamp(m_depth.stamp);
    }
    return true;
}

bool YarpRGBDFromDinrail::getImages(yarp::sig::FlexImage& colorFrame,
                                    yarp::sig::ImageOf<yarp::sig::PixelFloat>& depthFrame,
                                    yarp::os::Stamp* colorStamp,
                                    yarp::os::Stamp* depthStamp)
{
    if (!source().getImages(m_rgb, m_depth).ok() || !toYarpImage(m_rgb, colorFrame))
    {
        return false;
    }
    toYarpImage(m_depth, depthFrame);
    if (colorStamp != nullptr)
    {
        *colorStamp = toYarpStamp(m_rgb.stamp);
    }
    if (depthStamp != nullptr)
    {
        *depthStamp = toYarpStamp(m_depth.stamp);
    }
    return true;
}

yarp::dev::IRGBDSensor::RGBDSensor_status YarpRGBDFromDinrail::getSensorStatus()
{
    return toYarpSensorStatus(source().getSensorStatus());
}

// ---- dinrail::IRGBDSensor from yarp::dev::IRGBDSensor ----

using DinrailRGBDFromYarp = InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>;

RGBDSensorStatus DinrailRGBDFromYarp::getSensorStatus()
{
    return toDinrailSensorStatus(source().getSensorStatus());
}

std::string DinrailRGBDFromYarp::getLastErrorMessage()
{
    return source().getLastErrorMsg();
}

Status DinrailRGBDFromYarp::getRgbIntrinsics(CameraIntrinsics& intrinsics)
{
    yarp::os::Property property;
    if (!source().getRgbIntrinsicParam(property))
    {
        return Status(StatusCode::Unavailable);
    }
    intrinsics = toDinrailIntrinsics(property, source().getRgbWidth(), source().getRgbHeight());
    return Status();
}

Status DinrailRGBDFromYarp::getDepthIntrinsics(CameraIntrinsics& intrinsics)
{
    yarp::os::Property property;
    if (!source().getDepthIntrinsicParam(property))
    {
        return Status(StatusCode::Unavailable);
    }
    intrinsics = toDinrailIntrinsics(property, source().getDepthWidth(), source().getDepthHeight());
    return Status();
}

Status DinrailRGBDFromYarp::getDepthToRgbExtrinsics(CameraExtrinsics& extrinsics)
{
    yarp::sig::Matrix matrix(4, 4);
    if (!source().getExtrinsicParam(matrix) || !toDinrailExtrinsics(matrix, extrinsics))
    {
        return Status(StatusCode::Unavailable);
    }
    return Status();
}

Status DinrailRGBDFromYarp::getDepthClipPlanes(double& nearPlane, double& farPlane)
{
    return source().getDepthClipPlanes(nearPlane, farPlane) ? Status()
                                                            : Status(StatusCode::Unavailable);
}

Status DinrailRGBDFromYarp::getRgbImage(Image& rgbImage)
{
    if (!source().getRgbImage(m_rgb, &m_rgbStamp))
    {
        return Status(StatusCode::Unavailable);
    }
    return toDinrailImage(m_rgb, m_rgbStamp, rgbImage) ? Status()
                                                       : Status(StatusCode::Unimplemented);
}

Status DinrailRGBDFromYarp::getDepthImage(DepthImage& depthImage)
{
    if (!source().getDepthImage(m_depth, &m_depthStamp))
    {
        return Status(StatusCode::Unavailable);
    }
    toDinrailImage(m_depth, m_depthStamp, depthImage);
    return Status();
}

Status DinrailRGBDFromYarp::getImages(Image& rgbImage, DepthImage& depthImage)
{
    if (!source().getImages(m_rgb, m_depth, &m_rgbStamp, &m_depthStamp))
    {
        return Status(StatusCode::Unavailable);
    }
    if (!toDinrailImage(m_rgb, m_rgbStamp, rgbImage))
    {
        return Status(StatusCode::Unimplemented);
    }
    toDinrailImage(m_depth, m_depthStamp, depthImage);
    return Status();
}

// ---- yarp::dev::IFrameGrabberControls from dinrail::ICameraControls ----

using YarpControlsFromDinrail
    = InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::ICameraControls>;

std::optional<YarpControlsFromDinrail::Feature> YarpControlsFromDinrail::manualOption(int feature)
{
    switch (feature)
    {
    case YARP_FEATURE_BRIGHTNESS:
        return Feature{CameraSensor::Rgb, CameraOption::Brightness};
    case YARP_FEATURE_EXPOSURE:
        return Feature{CameraSensor::Rgb, CameraOption::Exposure};
    case YARP_FEATURE_SHARPNESS:
        return Feature{CameraSensor::Rgb, CameraOption::Sharpness};
    case YARP_FEATURE_WHITE_BALANCE:
        return Feature{CameraSensor::Rgb, CameraOption::WhiteBalance};
    case YARP_FEATURE_HUE:
        return Feature{CameraSensor::Rgb, CameraOption::Hue};
    case YARP_FEATURE_SATURATION:
        return Feature{CameraSensor::Rgb, CameraOption::Saturation};
    case YARP_FEATURE_GAMMA:
        return Feature{CameraSensor::Rgb, CameraOption::Gamma};
    case YARP_FEATURE_GAIN:
        return Feature{CameraSensor::Rgb, CameraOption::Gain};
    default:
        return std::nullopt;
    }
}

std::optional<YarpControlsFromDinrail::Feature>
YarpControlsFromDinrail::automaticOption(int feature)
{
    switch (feature)
    {
    case YARP_FEATURE_EXPOSURE:
    case YARP_FEATURE_GAIN:
        return Feature{CameraSensor::Rgb, CameraOption::AutoExposure};
    case YARP_FEATURE_WHITE_BALANCE:
        return Feature{CameraSensor::Rgb, CameraOption::AutoWhiteBalance};
    default:
        return std::nullopt;
    }
}

bool YarpControlsFromDinrail::getCameraDescription(CameraDescriptor* camera)
{
    if (camera == nullptr)
    {
        return false;
    }
    camera->busType = BUS_UNKNOWN;
    camera->deviceDescription = "dinrail camera";
    return true;
}

bool YarpControlsFromDinrail::hasFeature(int feature, bool* hasFeature)
{
    if (hasFeature == nullptr)
    {
        return false;
    }
    const auto option = manualOption(feature);
    *hasFeature = option && source().supportsOption(option->sensor, option->option);
    return true;
}

bool YarpControlsFromDinrail::setFeature(int feature, double value)
{
    const auto option = manualOption(feature);
    CameraOptionRange range;
    return option && source().getOptionRange(option->sensor, option->option, range).ok()
           && source().setOption(option->sensor, option->option, fromNormalized(value, range)).ok();
}

bool YarpControlsFromDinrail::getFeature(int feature, double* value)
{
    const auto option = manualOption(feature);
    CameraOptionRange range;
    double nativeValue = 0.0;
    if (value == nullptr || !option
        || !source().getOptionRange(option->sensor, option->option, range).ok()
        || !source().getOption(option->sensor, option->option, nativeValue).ok())
    {
        return false;
    }
    *value = toNormalized(nativeValue, range);
    return true;
}

bool YarpControlsFromDinrail::setFeature(int, double, double)
{
    // No dinrail option is described by two values.
    return false;
}

bool YarpControlsFromDinrail::getFeature(int, double*, double*)
{
    return false;
}

bool YarpControlsFromDinrail::hasOnOff(int, bool* HasOnOff)
{
    if (HasOnOff == nullptr)
    {
        return false;
    }
    *HasOnOff = false;
    return true;
}

bool YarpControlsFromDinrail::setActive(int, bool)
{
    return false;
}

bool YarpControlsFromDinrail::getActive(int feature, bool* isActive)
{
    bool supported = false;
    if (isActive == nullptr || !hasFeature(feature, &supported))
    {
        return false;
    }
    *isActive = supported;
    return true;
}

bool YarpControlsFromDinrail::hasAuto(int feature, bool* hasAuto)
{
    if (hasAuto == nullptr)
    {
        return false;
    }
    const auto option = automaticOption(feature);
    *hasAuto = option && source().supportsOption(option->sensor, option->option);
    return true;
}

bool YarpControlsFromDinrail::hasManual(int feature, bool* hasManual)
{
    return hasFeature(feature, hasManual);
}

bool YarpControlsFromDinrail::hasOnePush(int, bool* hasOnePush)
{
    if (hasOnePush == nullptr)
    {
        return false;
    }
    *hasOnePush = false;
    return true;
}

bool YarpControlsFromDinrail::setMode(int feature, FeatureMode mode)
{
    const auto option = automaticOption(feature);
    if (!option || mode == MODE_UNKNOWN)
    {
        return false;
    }
    return source().setOption(option->sensor, option->option, mode == MODE_AUTO ? 1.0 : 0.0).ok();
}

bool YarpControlsFromDinrail::getMode(int feature, FeatureMode* mode)
{
    if (mode == nullptr)
    {
        return false;
    }
    const auto option = automaticOption(feature);
    double value = 0.0;
    if (option && source().getOption(option->sensor, option->option, value).ok())
    {
        *mode = value != 0.0 ? MODE_AUTO : MODE_MANUAL;
        return true;
    }
    bool manual = false;
    if (hasFeature(feature, &manual) && manual)
    {
        *mode = MODE_MANUAL;
        return true;
    }
    return false;
}

bool YarpControlsFromDinrail::setOnePush(int)
{
    return false;
}

} // namespace dinrail
