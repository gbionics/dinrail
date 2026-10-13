// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <cstring>
#include <dinrail/YarpCameraAdapters.h>
#include <dinrail/YarpPropertyConverter.h>
#include <dinrail/YarpStampConverter.h>
#include <type_traits>
namespace dinrail
{
namespace
{
Status fromBool(bool ok)
{
    return ok ? OkStatus() : Status{StatusCode::Unknown};
}

// We deliberately implement these small conversions here and do not reuse
// OpenCV/YARP_cv color conversions as they can start parallel workers or
// allocate temporary buffers.
// Serial row copies and explicit RGB/BGR/mono conversion avoid those side effects
// and reuse storage after warmup. Honor both libraries' row strides, preserve RGB
// channel order, and copy into caller-owned outputs rather than exposing views.

void copyRows(const unsigned char* src,
              size_t srcStride,
              unsigned char* dst,
              size_t dstStride,
              size_t rowBytes,
              int rows)
{
    for (int row = 0; row < rows; ++row)
    {
        std::memcpy(dst + row * dstStride, src + row * srcStride, rowBytes);
    }
}

void copyMat(const cv::Mat& src, cv::Mat& dst)
{
    dst.create(src.rows, src.cols, src.type());
    copyRows(src.data, src.step[0], dst.data, dst.step[0], src.cols * src.elemSize(), src.rows);
}

template <class Pixel> constexpr int cvPixelType()
{
    if constexpr (std::is_same_v<Pixel, yarp::sig::PixelRgb>)
    {
        return CV_8UC3;
    } else if constexpr (std::is_same_v<Pixel, yarp::sig::PixelMono>)
    {
        return CV_8UC1;
    } else
    {
        static_assert(std::is_same_v<Pixel, yarp::sig::PixelFloat>);
        return CV_32FC1;
    }
}

template <class Pixel, class Image> bool copyToYarp(const cv::Mat& src, Image& dst)
{
    if (src.empty() || src.dims != 2 || src.type() != cvPixelType<Pixel>())
    {
        return false;
    }

    if constexpr (std::is_same_v<Image, yarp::sig::FlexImage>)
    {
        static_assert(std::is_same_v<Pixel, yarp::sig::PixelRgb>);
        dst.setPixelCode(VOCAB_PIXEL_RGB);
    }
    dst.resize(src.cols, src.rows);
    const size_t rowBytes = src.cols * src.elemSize();
    copyRows(src.data, src.step[0], dst.getRawImage(), dst.getRowSize(), rowBytes, src.rows);
    // YARP transmits row padding too; do not publish uninitialized padding bytes.
    for (int row = 0; row < src.rows; ++row)
    {
        std::memset(dst.getRawImage() + row * dst.getRowSize() + rowBytes,
                    0,
                    dst.getRowSize() - rowBytes);
    }
    return true;
}

template <class Pixel> Status copyToCv(const yarp::sig::ImageOf<Pixel>& src, cv::Mat& dst)
{
    if (src.width() == 0 || src.height() == 0)
    {
        return Status{StatusCode::InvalidArgument};
    }
    dst.create(src.height(), src.width(), cvPixelType<Pixel>());
    copyRows(src.getRawImage(),
             src.getRowSize(),
             dst.data,
             dst.step[0],
             src.width() * dst.elemSize(),
             dst.rows);
    return OkStatus();
}

Status copyRgbToCv(const yarp::sig::FlexImage& src, cv::Mat& dst)
{
    const int code = src.getPixelCode();
    if (code != VOCAB_PIXEL_RGB && code != VOCAB_PIXEL_BGR && code != VOCAB_PIXEL_MONO)
    {
        return Status{StatusCode::Unimplemented};
    }
    if (src.width() == 0 || src.height() == 0)
    {
        return Status{StatusCode::InvalidArgument};
    }

    dst.create(src.height(), src.width(), CV_8UC3);
    if (code == VOCAB_PIXEL_RGB)
    {
        copyRows(src.getRawImage(),
                 src.getRowSize(),
                 dst.data,
                 dst.step[0],
                 src.width() * dst.elemSize(),
                 dst.rows);
        return OkStatus();
    }
    for (int row = 0; row < dst.rows; ++row)
    {
        const auto* input = src.getRawImage() + row * src.getRowSize();
        auto* output = dst.ptr<cv::Vec3b>(row);
        for (int col = 0; col < dst.cols; ++col)
        {
            if (code == VOCAB_PIXEL_BGR)
            {
                output[col] = {input[3 * col + 2], input[3 * col + 1], input[3 * col]};
            } else
            {
                output[col] = {input[col], input[col], input[col]};
            }
        }
    }
    return OkStatus();
}
int pixelCode(PixelCoding coding)
{
    switch (coding)
    {
    case PixelCoding::RGB:
        return VOCAB_PIXEL_RGB;
    case PixelCoding::BGR:
        return VOCAB_PIXEL_BGR;
    case PixelCoding::Mono:
        return VOCAB_PIXEL_MONO;
    case PixelCoding::DepthFloat:
        return VOCAB_PIXEL_MONO_FLOAT;
    }
    return VOCAB_PIXEL_INVALID;
}
bool pixelCoding(int code, PixelCoding& coding)
{
    switch (code)
    {
    case VOCAB_PIXEL_RGB:
        coding = PixelCoding::RGB;
        return true;
    case VOCAB_PIXEL_BGR:
        coding = PixelCoding::BGR;
        return true;
    case VOCAB_PIXEL_MONO:
        coding = PixelCoding::Mono;
        return true;
    case VOCAB_PIXEL_MONO_FLOAT:
        coding = PixelCoding::DepthFloat;
        return true;
    default:
        return false;
    }
}
} // namespace

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls,
                      dinrail::IFrameGrabberControls>::getCameraDescription(::CameraDescriptor*
                                                                                camera)
{
    if (!camera)
    {
        return false;
    }
    dinrail::CameraDescriptor value;
    auto status = source().getCameraDescription(value);
    if (status.ok())
    {
        camera->busType = static_cast<::BusType>(value.busType);
        camera->deviceDescription = value.deviceDescription;
    }
    return status.ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::hasFeature(
    int feature, bool* hasFeature)
{
    if (!hasFeature)
    {
        return false;
    }
    return source().hasFeature(feature, *hasFeature).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::setFeature(
    int feature, double value)
{
    return source().setFeature(feature, value).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::getFeature(
    int feature, double* value)
{
    if (!value)
    {
        return false;
    }
    return source().getFeature(feature, *value).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::setFeature(
    int feature, double value1, double value2)
{
    return source().setFeature(feature, value1, value2).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::getFeature(
    int feature, double* value1, double* value2)
{
    if (!value1)
    {
        return false;
    }
    if (!value2)
    {
        return false;
    }
    return source().getFeature(feature, *value1, *value2).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::hasOnOff(
    int feature, bool* hasOnOff)
{
    if (!hasOnOff)
    {
        return false;
    }
    return source().hasOnOff(feature, *hasOnOff).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::setActive(
    int feature, bool onoff)
{
    return source().setActive(feature, onoff).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::getActive(
    int feature, bool* isActive)
{
    if (!isActive)
    {
        return false;
    }
    return source().getActive(feature, *isActive).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::hasAuto(
    int feature, bool* hasAuto)
{
    if (!hasAuto)
    {
        return false;
    }
    return source().hasAuto(feature, *hasAuto).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::hasManual(
    int feature, bool* hasManual)
{
    if (!hasManual)
    {
        return false;
    }
    return source().hasManual(feature, *hasManual).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::hasOnePush(
    int feature, bool* hasOnePush)
{
    if (!hasOnePush)
    {
        return false;
    }
    return source().hasOnePush(feature, *hasOnePush).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::setMode(
    int feature, ::FeatureMode mode)
{
    return source().setMode(feature, static_cast<dinrail::FeatureMode>(mode)).ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::getMode(
    int feature, ::FeatureMode* mode)
{
    if (!mode)
    {
        return false;
    }
    dinrail::FeatureMode value;
    auto status = source().getMode(feature, value);
    if (status.ok())
    {
        *mode = static_cast<::FeatureMode>(value);
    }
    return status.ok();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>::setOnePush(
    int feature)
{
    return source().setOnePush(feature).ok();
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls,
                 yarp::dev::IFrameGrabberControls>::getCameraDescription(CameraDescriptor& camera)
{
    ::CameraDescriptor value;
    if (!source().getCameraDescription(&value))
    {
        return Status{StatusCode::Unknown};
    }
    camera = {static_cast<dinrail::BusType>(value.busType), value.deviceDescription};
    return OkStatus();
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::hasFeature(
    int feature, bool& hasFeature)
{
    return fromBool(source().hasFeature(feature, &hasFeature));
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::setFeature(
    int feature, double value)
{
    return fromBool(source().setFeature(feature, value));
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::getFeature(
    int feature, double& value)
{
    return fromBool(source().getFeature(feature, &value));
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::setFeature(
    int feature, double value1, double value2)
{
    return fromBool(source().setFeature(feature, value1, value2));
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::getFeature(
    int feature, double& value1, double& value2)
{
    return fromBool(source().getFeature(feature, &value1, &value2));
}

Status InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::hasOnOff(
    int feature, bool& hasOnOff)
{
    return fromBool(source().hasOnOff(feature, &hasOnOff));
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::setActive(
    int feature, bool onoff)
{
    return fromBool(source().setActive(feature, onoff));
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::getActive(
    int feature, bool& isActive)
{
    return fromBool(source().getActive(feature, &isActive));
}

Status InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::hasAuto(
    int feature, bool& hasAuto)
{
    return fromBool(source().hasAuto(feature, &hasAuto));
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::hasManual(
    int feature, bool& hasManual)
{
    return fromBool(source().hasManual(feature, &hasManual));
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::hasOnePush(
    int feature, bool& hasOnePush)
{
    return fromBool(source().hasOnePush(feature, &hasOnePush));
}

Status InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::setMode(
    int feature, FeatureMode mode)
{
    return fromBool(source().setMode(feature, static_cast<::FeatureMode>(mode)));
}

Status InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::getMode(
    int feature, FeatureMode& mode)
{
    ::FeatureMode value;
    if (!source().getMode(feature, &value))
    {
        return Status{StatusCode::Unknown};
    }
    mode = static_cast<dinrail::FeatureMode>(value);
    return OkStatus();
}

Status
InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>::setOnePush(
    int feature)
{
    return fromBool(source().setOnePush(feature));
}

int InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>::height()
    const
{
    return source().height();
}

int InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>::width() const
{
    return source().width();
}

bool InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>::getImage(
    yarp::sig::ImageOf<yarp::sig::PixelMono>& image)
{
    const std::lock_guard lock(m_imageMutex);
    auto& first = m_image;
    auto status = source().getImage(first);
    if (!status.ok())
    {
        return false;
    }
    if (!copyToYarp<yarp::sig::PixelMono>(first, image))
    {
        return false;
    }
    return true;
}

int InterfaceAdapter<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>::height()
    const
{
    return source().height();
}

int InterfaceAdapter<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>::width() const
{
    return source().width();
}

Status InterfaceAdapter<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>::getImage(
    cv::Mat& image)
{
    const std::lock_guard lock(m_imageMutex);
    auto& first = m_image;
    if (!source().getImage(first))
    {
        return Status{StatusCode::Unknown};
    }
    auto status = copyToCv(first, image);
    if (!status.ok())
    {
        return status;
    }
    return OkStatus();
}

int InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::getRgbHeight()
{
    return source().getRgbHeight();
}

int InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::getRgbWidth()
{
    return source().getRgbWidth();
}

bool InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::
    getRgbSupportedConfigurations(yarp::sig::VectorOf<yarp::dev::CameraConfig>& configurations)
{
    std::vector<dinrail::CameraConfig> value;
    auto status = source().getRgbSupportedConfigurations(value);
    if (!status.ok())
    {
        return false;
    }
    yarp::sig::VectorOf<yarp::dev::CameraConfig> result;
    result.resize(value.size());
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        result[i] = {value[i].width,
                     value[i].height,
                     value[i].framerate,
                     static_cast<YarpVocabPixelTypesEnum>(pixelCode(value[i].pixelCoding))};
    }
    configurations = std::move(result);
    return true;
}

bool InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::getRgbResolution(
    int& width, int& height)
{
    return source().getRgbResolution(width, height).ok();
}

bool InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::setRgbResolution(
    int width, int height)
{
    return source().setRgbResolution(width, height).ok();
}

bool InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::getRgbFOV(
    double& horizontalFov, double& verticalFov)
{
    return source().getRgbFOV(horizontalFov, verticalFov).ok();
}

bool InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::setRgbFOV(
    double horizontalFov, double verticalFov)
{
    return source().setRgbFOV(horizontalFov, verticalFov).ok();
}

bool InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::getRgbIntrinsicParam(
    yarp::os::Property& intrinsic)
{
    Parameters value;
    auto status = source().getRgbIntrinsicParam(value);
    if (status.ok())
    {
        intrinsic = YarpPropertyConverter::toYarpProperty(value);
    }
    return status.ok();
}

bool InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::getRgbMirroring(
    bool& mirror)
{
    return source().getRgbMirroring(mirror).ok();
}

bool InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>::setRgbMirroring(
    bool mirror)
{
    return source().setRgbMirroring(mirror).ok();
}

int InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::getRgbHeight()
{
    return source().getRgbHeight();
}

int InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::getRgbWidth()
{
    return source().getRgbWidth();
}

Status InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::
    getRgbSupportedConfigurations(std::vector<CameraConfig>& configurations)
{
    yarp::sig::VectorOf<yarp::dev::CameraConfig> value;
    if (!source().getRgbSupportedConfigurations(value))
    {
        return Status{StatusCode::Unknown};
    }
    std::vector<dinrail::CameraConfig> result;
    for (const auto& entry : value)
    {
        PixelCoding coding;
        if (!pixelCoding(entry.pixelCoding, coding))
        {
            return Status{StatusCode::Unimplemented};
        }
        result.push_back({entry.width, entry.height, entry.framerate, coding});
    }
    configurations = std::move(result);
    return OkStatus();
}

Status InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::getRgbResolution(
    int& width, int& height)
{
    return fromBool(source().getRgbResolution(width, height));
}

Status InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::setRgbResolution(
    int width, int height)
{
    return fromBool(source().setRgbResolution(width, height));
}

Status InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::getRgbFOV(
    double& horizontalFov, double& verticalFov)
{
    return fromBool(source().getRgbFOV(horizontalFov, verticalFov));
}

Status InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::setRgbFOV(
    double horizontalFov, double verticalFov)
{
    return fromBool(source().setRgbFOV(horizontalFov, verticalFov));
}

Status
InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::getRgbIntrinsicParam(
    Parameters& intrinsic)
{
    yarp::os::Property value;
    if (!source().getRgbIntrinsicParam(value))
    {
        return Status{StatusCode::Unknown};
    }
    intrinsic = YarpPropertyConverter::toDinrailParameters(value);
    return OkStatus();
}

Status InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::getRgbMirroring(
    bool& mirror)
{
    return fromBool(source().getRgbMirroring(mirror));
}

Status InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>::setRgbMirroring(
    bool mirror)
{
    return fromBool(source().setRgbMirroring(mirror));
}

int InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>::getDepthHeight()
{
    return source().getDepthHeight();
}

int InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>::getDepthWidth()
{
    return source().getDepthWidth();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams,
                      dinrail::IDepthVisualParams>::setDepthResolution(int width, int height)
{
    return source().setDepthResolution(width, height).ok();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>::getDepthFOV(
    double& horizontalFov, double& verticalFov)
{
    return source().getDepthFOV(horizontalFov, verticalFov).ok();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>::setDepthFOV(
    double horizontalFov, double verticalFov)
{
    return source().setDepthFOV(horizontalFov, verticalFov).ok();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams,
                      dinrail::IDepthVisualParams>::getDepthIntrinsicParam(yarp::os::Property&
                                                                               intrinsic)
{
    Parameters value;
    auto status = source().getDepthIntrinsicParam(value);
    if (status.ok())
    {
        intrinsic = YarpPropertyConverter::toYarpProperty(value);
    }
    return status.ok();
}

double
InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>::getDepthAccuracy()
{
    return source().getDepthAccuracy();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>::setDepthAccuracy(
    double accuracy)
{
    return source().setDepthAccuracy(accuracy).ok();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams,
                      dinrail::IDepthVisualParams>::getDepthClipPlanes(double& nearPlane,
                                                                       double& farPlane)
{
    return source().getDepthClipPlanes(nearPlane, farPlane).ok();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams,
                      dinrail::IDepthVisualParams>::setDepthClipPlanes(double nearPlane,
                                                                       double farPlane)
{
    return source().setDepthClipPlanes(nearPlane, farPlane).ok();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>::getDepthMirroring(
    bool& mirror)
{
    return source().getDepthMirroring(mirror).ok();
}

bool InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>::setDepthMirroring(
    bool mirror)
{
    return source().setDepthMirroring(mirror).ok();
}

int InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::getDepthHeight()
{
    return source().getDepthHeight();
}

int InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::getDepthWidth()
{
    return source().getDepthWidth();
}

Status
InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::setDepthResolution(
    int width, int height)
{
    return fromBool(source().setDepthResolution(width, height));
}

Status InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::getDepthFOV(
    double& horizontalFov, double& verticalFov)
{
    return fromBool(source().getDepthFOV(horizontalFov, verticalFov));
}

Status InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::setDepthFOV(
    double horizontalFov, double verticalFov)
{
    return fromBool(source().setDepthFOV(horizontalFov, verticalFov));
}

Status
InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::getDepthIntrinsicParam(
    Parameters& intrinsic)
{
    yarp::os::Property value;
    if (!source().getDepthIntrinsicParam(value))
    {
        return Status{StatusCode::Unknown};
    }
    intrinsic = YarpPropertyConverter::toDinrailParameters(value);
    return OkStatus();
}

double
InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::getDepthAccuracy()
{
    return source().getDepthAccuracy();
}

Status
InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::setDepthAccuracy(
    double accuracy)
{
    return fromBool(source().setDepthAccuracy(accuracy));
}

Status
InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::getDepthClipPlanes(
    double& nearPlane, double& farPlane)
{
    return fromBool(source().getDepthClipPlanes(nearPlane, farPlane));
}

Status
InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::setDepthClipPlanes(
    double nearPlane, double farPlane)
{
    return fromBool(source().setDepthClipPlanes(nearPlane, farPlane));
}

Status
InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::getDepthMirroring(
    bool& mirror)
{
    return fromBool(source().getDepthMirroring(mirror));
}

Status
InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>::setDepthMirroring(
    bool mirror)
{
    return fromBool(source().setDepthMirroring(mirror));
}

int InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getRgbHeight()
{
    return source().getRgbHeight();
}

int InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getRgbWidth()
{
    return source().getRgbWidth();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getRgbSupportedConfigurations(
    yarp::sig::VectorOf<yarp::dev::CameraConfig>& configurations)
{
    std::vector<dinrail::CameraConfig> value;
    auto status = source().getRgbSupportedConfigurations(value);
    if (!status.ok())
    {
        return false;
    }
    yarp::sig::VectorOf<yarp::dev::CameraConfig> result;
    result.resize(value.size());
    for (std::size_t i = 0; i < value.size(); ++i)
    {
        result[i] = {value[i].width,
                     value[i].height,
                     value[i].framerate,
                     static_cast<YarpVocabPixelTypesEnum>(pixelCode(value[i].pixelCoding))};
    }
    configurations = std::move(result);
    return true;
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getRgbResolution(int& width,
                                                                                      int& height)
{
    return source().getRgbResolution(width, height).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::setRgbResolution(int width,
                                                                                      int height)
{
    return source().setRgbResolution(width, height).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getRgbFOV(
    double& horizontalFov, double& verticalFov)
{
    return source().getRgbFOV(horizontalFov, verticalFov).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::setRgbFOV(double horizontalFov,
                                                                               double verticalFov)
{
    return source().setRgbFOV(horizontalFov, verticalFov).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getRgbIntrinsicParam(
    yarp::os::Property& intrinsic)
{
    Parameters value;
    auto status = source().getRgbIntrinsicParam(value);
    if (status.ok())
    {
        intrinsic = YarpPropertyConverter::toYarpProperty(value);
    }
    return status.ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getRgbMirroring(bool& mirror)
{
    return source().getRgbMirroring(mirror).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::setRgbMirroring(bool mirror)
{
    return source().setRgbMirroring(mirror).ok();
}

int InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getDepthHeight()
{
    return source().getDepthHeight();
}

int InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getDepthWidth()
{
    return source().getDepthWidth();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::setDepthResolution(int width,
                                                                                        int height)
{
    return source().setDepthResolution(width, height).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getDepthFOV(
    double& horizontalFov, double& verticalFov)
{
    return source().getDepthFOV(horizontalFov, verticalFov).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::setDepthFOV(
    double horizontalFov, double verticalFov)
{
    return source().setDepthFOV(horizontalFov, verticalFov).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getDepthIntrinsicParam(
    yarp::os::Property& intrinsic)
{
    Parameters value;
    auto status = source().getDepthIntrinsicParam(value);
    if (status.ok())
    {
        intrinsic = YarpPropertyConverter::toYarpProperty(value);
    }
    return status.ok();
}

double InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getDepthAccuracy()
{
    return source().getDepthAccuracy();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::setDepthAccuracy(
    double accuracy)
{
    return source().setDepthAccuracy(accuracy).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getDepthClipPlanes(
    double& nearPlane, double& farPlane)
{
    return source().getDepthClipPlanes(nearPlane, farPlane).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::setDepthClipPlanes(
    double nearPlane, double farPlane)
{
    return source().setDepthClipPlanes(nearPlane, farPlane).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getDepthMirroring(bool& mirror)
{
    return source().getDepthMirroring(mirror).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::setDepthMirroring(bool mirror)
{
    return source().setDepthMirroring(mirror).ok();
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getExtrinsicParam(
    yarp::sig::Matrix& extrinsic)
{
    cv::Mat value;
    if (!source().getExtrinsicParam(value).ok() || value.rows != 4 || value.cols != 4
        || value.type() != CV_64FC1)
    {
        return false;
    }
    extrinsic.resize(4, 4);
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            extrinsic(r, c) = value.at<double>(r, c);
        }
    }
    return true;
}

std::string InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getLastErrorMsg(
    yarp::os::Stamp* timeStamp)
{
    dinrail::Stamp stamp;
    auto result = source().getLastErrorMsg(timeStamp ? &stamp : nullptr);
    if (timeStamp)
    {
        *timeStamp = toYarpStamp(stamp);
    }
    return result;
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getRgbImage(
    yarp::sig::FlexImage& rgbImage, yarp::os::Stamp* timeStamp)
{
    const std::lock_guard lock(m_imageMutex);
    auto& first = m_rgb;
    dinrail::Stamp firstStamp;
    auto status = source().getRgbImage(first, timeStamp ? &firstStamp : nullptr);
    if (!status.ok())
    {
        return false;
    }
    if (!copyToYarp<yarp::sig::PixelRgb>(first, rgbImage))
    {
        return false;
    }
    if (timeStamp)
    {
        *timeStamp = toYarpStamp(firstStamp);
    }
    return true;
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getDepthImage(
    yarp::sig::ImageOf<yarp::sig::PixelFloat>& depthImage, yarp::os::Stamp* timeStamp)
{
    const std::lock_guard lock(m_imageMutex);
    auto& first = m_depth;
    dinrail::Stamp firstStamp;
    auto status = source().getDepthImage(first, timeStamp ? &firstStamp : nullptr);
    if (!status.ok())
    {
        return false;
    }
    if (!copyToYarp<yarp::sig::PixelFloat>(first, depthImage))
    {
        return false;
    }
    if (timeStamp)
    {
        *timeStamp = toYarpStamp(firstStamp);
    }
    return true;
}

bool InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getImages(
    yarp::sig::FlexImage& colorFrame,
    yarp::sig::ImageOf<yarp::sig::PixelFloat>& depthFrame,
    yarp::os::Stamp* colorStamp,
    yarp::os::Stamp* depthStamp)
{
    const std::lock_guard lock(m_imageMutex);
    auto& first = m_rgb;
    auto& second = m_depth;
    dinrail::Stamp firstStamp, secondStamp;
    auto status = source().getImages(first,
                                     second,
                                     colorStamp ? &firstStamp : nullptr,
                                     depthStamp ? &secondStamp : nullptr);
    if (!status.ok())
    {
        return false;
    }
    if (!copyToYarp<yarp::sig::PixelRgb>(first, colorFrame))
    {
        return false;
    }
    if (!copyToYarp<yarp::sig::PixelFloat>(second, depthFrame))
    {
        return false;
    }
    if (colorStamp)
    {
        *colorStamp = toYarpStamp(firstStamp);
    }
    if (depthStamp)
    {
        *depthStamp = toYarpStamp(secondStamp);
    }
    return true;
}

yarp::dev::IRGBDSensor::RGBDSensor_status
InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>::getSensorStatus()
{
    return static_cast<yarp::dev::IRGBDSensor::RGBDSensor_status>(source().getSensorStatus());
}

int InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getRgbHeight()
{
    return source().getRgbHeight();
}

int InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getRgbWidth()
{
    return source().getRgbWidth();
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getRgbSupportedConfigurations(
    std::vector<CameraConfig>& configurations)
{
    yarp::sig::VectorOf<yarp::dev::CameraConfig> value;
    if (!source().getRgbSupportedConfigurations(value))
    {
        return Status{StatusCode::Unknown};
    }
    std::vector<dinrail::CameraConfig> result;
    for (const auto& entry : value)
    {
        PixelCoding coding;
        if (!pixelCoding(entry.pixelCoding, coding))
        {
            return Status{StatusCode::Unimplemented};
        }
        result.push_back({entry.width, entry.height, entry.framerate, coding});
    }
    configurations = std::move(result);
    return OkStatus();
}

Status InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getRgbResolution(int& width,
                                                                                        int& height)
{
    return fromBool(source().getRgbResolution(width, height));
}

Status InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::setRgbResolution(int width,
                                                                                        int height)
{
    return fromBool(source().setRgbResolution(width, height));
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getRgbFOV(double& horizontalFov,
                                                                          double& verticalFov)
{
    return fromBool(source().getRgbFOV(horizontalFov, verticalFov));
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::setRgbFOV(double horizontalFov,
                                                                          double verticalFov)
{
    return fromBool(source().setRgbFOV(horizontalFov, verticalFov));
}

Status InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getRgbIntrinsicParam(
    Parameters& intrinsic)
{
    yarp::os::Property value;
    if (!source().getRgbIntrinsicParam(value))
    {
        return Status{StatusCode::Unknown};
    }
    intrinsic = YarpPropertyConverter::toDinrailParameters(value);
    return OkStatus();
}

Status InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getRgbMirroring(bool& mirror)
{
    return fromBool(source().getRgbMirroring(mirror));
}

Status InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::setRgbMirroring(bool mirror)
{
    return fromBool(source().setRgbMirroring(mirror));
}

int InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getDepthHeight()
{
    return source().getDepthHeight();
}

int InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getDepthWidth()
{
    return source().getDepthWidth();
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::setDepthResolution(int width,
                                                                                   int height)
{
    return fromBool(source().setDepthResolution(width, height));
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getDepthFOV(double& horizontalFov,
                                                                            double& verticalFov)
{
    return fromBool(source().getDepthFOV(horizontalFov, verticalFov));
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::setDepthFOV(double horizontalFov,
                                                                            double verticalFov)
{
    return fromBool(source().setDepthFOV(horizontalFov, verticalFov));
}

Status InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getDepthIntrinsicParam(
    Parameters& intrinsic)
{
    yarp::os::Property value;
    if (!source().getDepthIntrinsicParam(value))
    {
        return Status{StatusCode::Unknown};
    }
    intrinsic = YarpPropertyConverter::toDinrailParameters(value);
    return OkStatus();
}

double InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getDepthAccuracy()
{
    return source().getDepthAccuracy();
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::setDepthAccuracy(double accuracy)
{
    return fromBool(source().setDepthAccuracy(accuracy));
}

Status InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getDepthClipPlanes(
    double& nearPlane, double& farPlane)
{
    return fromBool(source().getDepthClipPlanes(nearPlane, farPlane));
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::setDepthClipPlanes(double nearPlane,
                                                                                   double farPlane)
{
    return fromBool(source().setDepthClipPlanes(nearPlane, farPlane));
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getDepthMirroring(bool& mirror)
{
    return fromBool(source().getDepthMirroring(mirror));
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::setDepthMirroring(bool mirror)
{
    return fromBool(source().setDepthMirroring(mirror));
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getExtrinsicParam(cv::Mat& extrinsic)
{
    yarp::sig::Matrix value;
    if (!source().getExtrinsicParam(value))
    {
        return Status{StatusCode::Unknown};
    }
    if (value.rows() != 4 || value.cols() != 4)
    {
        return Status{StatusCode::InvalidArgument};
    }
    cv::Mat result(4, 4, CV_64FC1);
    for (int r = 0; r < 4; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            result.at<double>(r, c) = value(r, c);
        }
    }
    extrinsic = std::move(result);
    return OkStatus();
}

std::string
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getLastErrorMsg(Stamp* timeStamp)
{
    yarp::os::Stamp stamp;
    auto result = source().getLastErrorMsg(timeStamp ? &stamp : nullptr);
    if (timeStamp)
    {
        *timeStamp = toDinrailStamp(stamp);
    }
    return result;
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getRgbImage(cv::Mat& rgbImage,
                                                                            Stamp* timeStamp)
{
    const std::lock_guard lock(m_imageMutex);
    auto& first = m_rgb;
    yarp::os::Stamp firstStamp;
    if (!source().getRgbImage(first, timeStamp ? &firstStamp : nullptr))
    {
        return Status{StatusCode::Unknown};
    }
    auto& resultFirst = m_rgbResult;
    auto status = copyRgbToCv(first, resultFirst);
    if (!status.ok())
    {
        return status;
    }
    if (timeStamp)
    {
        *timeStamp = toDinrailStamp(firstStamp);
    }
    copyMat(resultFirst, rgbImage);
    return OkStatus();
}

Status
InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getDepthImage(cv::Mat& depthImage,
                                                                              Stamp* timeStamp)
{
    const std::lock_guard lock(m_imageMutex);
    auto& first = m_depth;
    yarp::os::Stamp firstStamp;
    if (!source().getDepthImage(first, timeStamp ? &firstStamp : nullptr))
    {
        return Status{StatusCode::Unknown};
    }
    auto& resultFirst = m_depthResult;
    auto status = copyToCv(first, resultFirst);
    if (!status.ok())
    {
        return status;
    }
    if (timeStamp)
    {
        *timeStamp = toDinrailStamp(firstStamp);
    }
    copyMat(resultFirst, depthImage);
    return OkStatus();
}

Status InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getImages(
    cv::Mat& colorFrame, cv::Mat& depthFrame, Stamp* colorStamp, Stamp* depthStamp)
{
    const std::lock_guard lock(m_imageMutex);
    auto& first = m_rgb;
    auto& second = m_depth;
    yarp::os::Stamp firstStamp, secondStamp;
    if (!source().getImages(first,
                            second,
                            colorStamp ? &firstStamp : nullptr,
                            depthStamp ? &secondStamp : nullptr))
    {
        return Status{StatusCode::Unknown};
    }
    auto& resultFirst = m_rgbResult;
    auto& resultSecond = m_depthResult;
    auto status = copyRgbToCv(first, resultFirst);
    if (!status.ok())
    {
        return status;
    }
    status = copyToCv(second, resultSecond);
    if (!status.ok())
    {
        return status;
    }
    copyMat(resultSecond, depthFrame);
    if (colorStamp)
    {
        *colorStamp = toDinrailStamp(firstStamp);
    }
    if (depthStamp)
    {
        *depthStamp = toDinrailStamp(secondStamp);
    }
    copyMat(resultFirst, colorFrame);
    return OkStatus();
}

RGBDSensorStatus InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>::getSensorStatus()
{
    return static_cast<dinrail::RGBDSensorStatus>(source().getSensorStatus());
}

int InterfaceAdapter<yarp::dev::IFrameGrabberImage, dinrail::IRGBDSensor>::width() const
{
    return const_cast<dinrail::IRGBDSensor&>(source()).getRgbWidth();
}
int InterfaceAdapter<yarp::dev::IFrameGrabberImage, dinrail::IRGBDSensor>::height() const
{
    return const_cast<dinrail::IRGBDSensor&>(source()).getRgbHeight();
}
bool InterfaceAdapter<yarp::dev::IFrameGrabberImage, dinrail::IRGBDSensor>::getImage(
    yarp::sig::ImageOf<yarp::sig::PixelRgb>& image)
{
    const std::lock_guard lock(m_imageMutex);
    auto& value = m_image;
    return source().getRgbImage(value).ok() && copyToYarp<yarp::sig::PixelRgb>(value, image);
}
} // namespace dinrail
