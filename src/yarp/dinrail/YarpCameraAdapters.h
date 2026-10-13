// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPCAMERAADAPTERS_H
#define DINRAIL_YARPCAMERAADAPTERS_H
#include <dinrail/IFrameGrabberControls.h>
#include <dinrail/IFrameGrabberImageRaw.h>
#include <dinrail/IRGBDSensor.h>
#include <dinrail/InterfaceAdapter.h>
#include <mutex>
#include <yarp/dev/IFrameGrabberControls.h>
#include <yarp/dev/IFrameGrabberImage.h>
#include <yarp/dev/IRGBDSensor.h>
namespace dinrail
{
/** Camera adapters perform serial image conversions with independent storage,
 * preserve acquisition stamps and use RGB channel order for dinrail RGB images.
 * YARP BGR inputs are converted. Unsupported encodings return Unimplemented.
 *
 * Conversions do not change OpenCV's process-wide threading configuration or
 * use its parallel conversion routines. Image reads are serialized per adapter
 * and reuse acquisition/conversion buffers. At a fixed resolution, adapter-owned
 * storage is allocated on the first read only. Callers should reuse output images
 * to avoid allocating their storage each frame. Source devices may allocate or
 * use parallel processing independently. */
template <>
class InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>
    : public InterfaceAdapterBase<yarp::dev::IFrameGrabberControls, dinrail::IFrameGrabberControls>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool getCameraDescription(::CameraDescriptor* camera) override;
    bool hasFeature(int feature, bool* hasFeature) override;
    bool setFeature(int feature, double value) override;
    bool getFeature(int feature, double* value) override;
    bool setFeature(int feature, double value1, double value2) override;
    bool getFeature(int feature, double* value1, double* value2) override;
    bool hasOnOff(int feature, bool* hasOnOff) override;
    bool setActive(int feature, bool onoff) override;
    bool getActive(int feature, bool* isActive) override;
    bool hasAuto(int feature, bool* hasAuto) override;
    bool hasManual(int feature, bool* hasManual) override;
    bool hasOnePush(int feature, bool* hasOnePush) override;
    bool setMode(int feature, ::FeatureMode mode) override;
    bool getMode(int feature, ::FeatureMode* mode) override;
    bool setOnePush(int feature) override;
};
template <>
class InterfaceAdapter<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>
    : public InterfaceAdapterBase<dinrail::IFrameGrabberControls, yarp::dev::IFrameGrabberControls>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    Status getCameraDescription(CameraDescriptor& camera) override;
    Status hasFeature(int feature, bool& hasFeature) override;
    Status setFeature(int feature, double value) override;
    Status getFeature(int feature, double& value) override;
    Status setFeature(int feature, double value1, double value2) override;
    Status getFeature(int feature, double& value1, double& value2) override;
    Status hasOnOff(int feature, bool& hasOnOff) override;
    Status setActive(int feature, bool onoff) override;
    Status getActive(int feature, bool& isActive) override;
    Status hasAuto(int feature, bool& hasAuto) override;
    Status hasManual(int feature, bool& hasManual) override;
    Status hasOnePush(int feature, bool& hasOnePush) override;
    Status setMode(int feature, FeatureMode mode) override;
    Status getMode(int feature, FeatureMode& mode) override;
    Status setOnePush(int feature) override;
};
template <>
class InterfaceAdapter<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>
    : public InterfaceAdapterBase<yarp::dev::IFrameGrabberImageRaw, dinrail::IFrameGrabberImageRaw>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int height() const override;
    int width() const override;
    bool getImage(yarp::sig::ImageOf<yarp::sig::PixelMono>& image) override;

private:
    std::mutex m_imageMutex;
    cv::Mat m_image;
};
template <>
class InterfaceAdapter<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>
    : public InterfaceAdapterBase<dinrail::IFrameGrabberImageRaw, yarp::dev::IFrameGrabberImageRaw>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int height() const override;
    int width() const override;
    Status getImage(cv::Mat& image) override;

private:
    std::mutex m_imageMutex;
    yarp::sig::ImageOf<yarp::sig::PixelMono> m_image;
};
template <>
class InterfaceAdapter<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>
    : public InterfaceAdapterBase<yarp::dev::IRgbVisualParams, dinrail::IRgbVisualParams>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int getRgbHeight() override;
    int getRgbWidth() override;
    bool getRgbSupportedConfigurations(
        yarp::sig::VectorOf<yarp::dev::CameraConfig>& configurations) override;
    bool getRgbResolution(int& width, int& height) override;
    bool setRgbResolution(int width, int height) override;
    bool getRgbFOV(double& horizontalFov, double& verticalFov) override;
    bool setRgbFOV(double horizontalFov, double verticalFov) override;
    bool getRgbIntrinsicParam(yarp::os::Property& intrinsic) override;
    bool getRgbMirroring(bool& mirror) override;
    bool setRgbMirroring(bool mirror) override;
};
template <>
class InterfaceAdapter<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>
    : public InterfaceAdapterBase<dinrail::IRgbVisualParams, yarp::dev::IRgbVisualParams>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int getRgbHeight() override;
    int getRgbWidth() override;
    Status getRgbSupportedConfigurations(std::vector<CameraConfig>& configurations) override;
    Status getRgbResolution(int& width, int& height) override;
    Status setRgbResolution(int width, int height) override;
    Status getRgbFOV(double& horizontalFov, double& verticalFov) override;
    Status setRgbFOV(double horizontalFov, double verticalFov) override;
    Status getRgbIntrinsicParam(Parameters& intrinsic) override;
    Status getRgbMirroring(bool& mirror) override;
    Status setRgbMirroring(bool mirror) override;
};
template <>
class InterfaceAdapter<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>
    : public InterfaceAdapterBase<yarp::dev::IDepthVisualParams, dinrail::IDepthVisualParams>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int getDepthHeight() override;
    int getDepthWidth() override;
    bool setDepthResolution(int width, int height) override;
    bool getDepthFOV(double& horizontalFov, double& verticalFov) override;
    bool setDepthFOV(double horizontalFov, double verticalFov) override;
    bool getDepthIntrinsicParam(yarp::os::Property& intrinsic) override;
    double getDepthAccuracy() override;
    bool setDepthAccuracy(double accuracy) override;
    bool getDepthClipPlanes(double& nearPlane, double& farPlane) override;
    bool setDepthClipPlanes(double nearPlane, double farPlane) override;
    bool getDepthMirroring(bool& mirror) override;
    bool setDepthMirroring(bool mirror) override;
};
template <>
class InterfaceAdapter<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>
    : public InterfaceAdapterBase<dinrail::IDepthVisualParams, yarp::dev::IDepthVisualParams>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int getDepthHeight() override;
    int getDepthWidth() override;
    Status setDepthResolution(int width, int height) override;
    Status getDepthFOV(double& horizontalFov, double& verticalFov) override;
    Status setDepthFOV(double horizontalFov, double verticalFov) override;
    Status getDepthIntrinsicParam(Parameters& intrinsic) override;
    double getDepthAccuracy() override;
    Status setDepthAccuracy(double accuracy) override;
    Status getDepthClipPlanes(double& nearPlane, double& farPlane) override;
    Status setDepthClipPlanes(double nearPlane, double farPlane) override;
    Status getDepthMirroring(bool& mirror) override;
    Status setDepthMirroring(bool mirror) override;
};
template <>
class InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>
    : public InterfaceAdapterBase<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int getRgbHeight() override;
    int getRgbWidth() override;
    bool getRgbSupportedConfigurations(
        yarp::sig::VectorOf<yarp::dev::CameraConfig>& configurations) override;
    bool getRgbResolution(int& width, int& height) override;
    bool setRgbResolution(int width, int height) override;
    bool getRgbFOV(double& horizontalFov, double& verticalFov) override;
    bool setRgbFOV(double horizontalFov, double verticalFov) override;
    bool getRgbIntrinsicParam(yarp::os::Property& intrinsic) override;
    bool getRgbMirroring(bool& mirror) override;
    bool setRgbMirroring(bool mirror) override;
    int getDepthHeight() override;
    int getDepthWidth() override;
    bool setDepthResolution(int width, int height) override;
    bool getDepthFOV(double& horizontalFov, double& verticalFov) override;
    bool setDepthFOV(double horizontalFov, double verticalFov) override;
    bool getDepthIntrinsicParam(yarp::os::Property& intrinsic) override;
    double getDepthAccuracy() override;
    bool setDepthAccuracy(double accuracy) override;
    bool getDepthClipPlanes(double& nearPlane, double& farPlane) override;
    bool setDepthClipPlanes(double nearPlane, double farPlane) override;
    bool getDepthMirroring(bool& mirror) override;
    bool setDepthMirroring(bool mirror) override;
    bool getExtrinsicParam(yarp::sig::Matrix& extrinsic) override;
    std::string getLastErrorMsg(yarp::os::Stamp* timeStamp = nullptr) override;
    bool getRgbImage(yarp::sig::FlexImage& rgbImage, yarp::os::Stamp* timeStamp = nullptr) override;
    bool getDepthImage(yarp::sig::ImageOf<yarp::sig::PixelFloat>& depthImage,
                       yarp::os::Stamp* timeStamp = nullptr) override;
    bool getImages(yarp::sig::FlexImage& colorFrame,
                   yarp::sig::ImageOf<yarp::sig::PixelFloat>& depthFrame,
                   yarp::os::Stamp* colorStamp = nullptr,
                   yarp::os::Stamp* depthStamp = nullptr) override;
    yarp::dev::IRGBDSensor::RGBDSensor_status getSensorStatus() override;

private:
    std::mutex m_imageMutex;
    cv::Mat m_rgb;
    cv::Mat m_depth;
};
template <>
class InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>
    : public InterfaceAdapterBase<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int getRgbHeight() override;
    int getRgbWidth() override;
    Status getRgbSupportedConfigurations(std::vector<CameraConfig>& configurations) override;
    Status getRgbResolution(int& width, int& height) override;
    Status setRgbResolution(int width, int height) override;
    Status getRgbFOV(double& horizontalFov, double& verticalFov) override;
    Status setRgbFOV(double horizontalFov, double verticalFov) override;
    Status getRgbIntrinsicParam(Parameters& intrinsic) override;
    Status getRgbMirroring(bool& mirror) override;
    Status setRgbMirroring(bool mirror) override;
    int getDepthHeight() override;
    int getDepthWidth() override;
    Status setDepthResolution(int width, int height) override;
    Status getDepthFOV(double& horizontalFov, double& verticalFov) override;
    Status setDepthFOV(double horizontalFov, double verticalFov) override;
    Status getDepthIntrinsicParam(Parameters& intrinsic) override;
    double getDepthAccuracy() override;
    Status setDepthAccuracy(double accuracy) override;
    Status getDepthClipPlanes(double& nearPlane, double& farPlane) override;
    Status setDepthClipPlanes(double nearPlane, double farPlane) override;
    Status getDepthMirroring(bool& mirror) override;
    Status setDepthMirroring(bool mirror) override;
    Status getExtrinsicParam(cv::Mat& extrinsic) override;
    std::string getLastErrorMsg(Stamp* timeStamp = nullptr) override;
    Status getRgbImage(cv::Mat& rgbImage, Stamp* timeStamp = nullptr) override;
    Status getDepthImage(cv::Mat& depthImage, Stamp* timeStamp = nullptr) override;
    Status getImages(cv::Mat& colorFrame,
                     cv::Mat& depthFrame,
                     Stamp* colorStamp = nullptr,
                     Stamp* depthStamp = nullptr) override;
    RGBDSensorStatus getSensorStatus() override;

private:
    std::mutex m_imageMutex;
    yarp::sig::FlexImage m_rgb;
    yarp::sig::ImageOf<yarp::sig::PixelFloat> m_depth;
    cv::Mat m_rgbResult;
    cv::Mat m_depthResult;
};
template <>
class InterfaceAdapter<yarp::dev::IFrameGrabberImage, dinrail::IRGBDSensor>
    : public InterfaceAdapterBase<yarp::dev::IFrameGrabberImage, dinrail::IRGBDSensor>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    int width() const override;
    int height() const override;
    bool getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb>& image) override;

private:
    std::mutex m_imageMutex;
    cv::Mat m_image;
};
} // namespace dinrail
#endif
