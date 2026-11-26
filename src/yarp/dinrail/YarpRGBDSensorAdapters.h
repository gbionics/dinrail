// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPRGBDSENSORADAPTERS_H
#define DINRAIL_YARPRGBDSENSORADAPTERS_H

#include <dinrail/ICameraControls.h>
#include <dinrail/IRGBDSensor.h>
#include <dinrail/InterfaceAdapter.h>

#include <yarp/dev/IFrameGrabberControls.h>
#include <yarp/dev/IRGBDSensor.h>
#include <yarp/os/Property.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Image.h>
#include <yarp/sig/Matrix.h>

#include <optional>

namespace dinrail
{

/** @name Conversions between the dinrail and the YARP RGBD types. */
///@{
/** @return false if the format of the image has no YARP equivalent. */
bool toYarpImage(const Image& image, yarp::sig::FlexImage& yarpImage);
/** @return false if the pixel code of the image has no dinrail equivalent. */
bool toDinrailImage(const yarp::sig::FlexImage& yarpImage,
                    const yarp::os::Stamp& stamp,
                    Image& image);
void toYarpImage(const DepthImage& image, yarp::sig::ImageOf<yarp::sig::PixelFloat>& yarpImage);
void toDinrailImage(const yarp::sig::ImageOf<yarp::sig::PixelFloat>& yarpImage,
                    const yarp::os::Stamp& stamp,
                    DepthImage& image);
/** Intrinsics in the format of yarp::dev::IRgbVisualParams::getRgbIntrinsicParam(). */
void toYarpIntrinsics(const CameraIntrinsics& intrinsics, yarp::os::Property& yarpIntrinsics);
CameraIntrinsics
toDinrailIntrinsics(const yarp::os::Property& yarpIntrinsics, int width, int height);
/** 4x4 homogeneous transformation, as yarp::dev::IRGBDSensor::getExtrinsicParam(). */
void toYarpExtrinsics(const CameraExtrinsics& extrinsics, yarp::sig::Matrix& yarpExtrinsics);
bool toDinrailExtrinsics(const yarp::sig::Matrix& yarpExtrinsics, CameraExtrinsics& extrinsics);
yarp::dev::IRGBDSensor::RGBDSensor_status toYarpSensorStatus(RGBDSensorStatus status);
RGBDSensorStatus toDinrailSensorStatus(yarp::dev::IRGBDSensor::RGBDSensor_status status);
/** Field of view [deg] of a pinhole camera. */
void fieldOfView(const CameraIntrinsics& intrinsics, double& horizontalFov, double& verticalFov);
///@}

/**
 * Expose a dinrail RGBD sensor as a YARP one. Images are copied in the YARP types, the
 * dinrail stamps become the YARP envelopes, and the image size, the field of view and the
 * intrinsic parameters are computed from the dinrail intrinsics. The resolution, the field of
 * view, the accuracy, the clip planes and the mirroring cannot be changed.
 */
template <>
class InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>
    : public InterfaceAdapterBase<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    int getRgbHeight() override;
    int getRgbWidth() override;
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
    RGBDSensor_status getSensorStatus() override;

private:
    Image m_rgb;
    DepthImage m_depth;
};

/**
 * Expose a YARP RGBD sensor as a dinrail one. The sequence numbers of the images are the
 * counters of their YARP envelopes.
 */
template <>
class InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>
    : public InterfaceAdapterBase<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    RGBDSensorStatus getSensorStatus() override;
    std::string getLastErrorMessage() override;
    Status getRgbIntrinsics(CameraIntrinsics& intrinsics) override;
    Status getDepthIntrinsics(CameraIntrinsics& intrinsics) override;
    Status getDepthToRgbExtrinsics(CameraExtrinsics& extrinsics) override;
    Status getDepthClipPlanes(double& nearPlane, double& farPlane) override;
    Status getRgbImage(Image& rgbImage) override;
    Status getDepthImage(DepthImage& depthImage) override;
    Status getImages(Image& rgbImage, DepthImage& depthImage) override;

private:
    yarp::sig::FlexImage m_rgb;
    yarp::sig::ImageOf<yarp::sig::PixelFloat> m_depth;
    // Persistent, as some YARP devices update the stamp they receive instead of overwriting it.
    yarp::os::Stamp m_rgbStamp;
    yarp::os::Stamp m_depthStamp;
};

/**
 * Expose dinrail camera controls as YARP frame grabber controls. As YARP expects, feature values
 * are normalized in [0, 1] over the range of the option, and the automatic mode of exposure,
 * gain and white balance is the corresponding automatic option. Features refer to the color
 * camera, except the ones that only the depth module provides.
 */
template <>
class InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::ICameraControls>
    : public InterfaceAdapterBase<yarp::dev::IFrameGrabberControls, dinrail::ICameraControls>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    bool getCameraDescription(CameraDescriptor* camera) override;
    bool hasFeature(int feature, bool* hasFeature) override;
    bool setFeature(int feature, double value) override;
    bool getFeature(int feature, double* value) override;
    bool setFeature(int feature, double value1, double value2) override;
    bool getFeature(int feature, double* value1, double* value2) override;
    bool hasOnOff(int feature, bool* HasOnOff) override;
    bool setActive(int feature, bool onoff) override;
    bool getActive(int feature, bool* isActive) override;
    bool hasAuto(int feature, bool* hasAuto) override;
    bool hasManual(int feature, bool* hasManual) override;
    bool hasOnePush(int feature, bool* hasOnePush) override;
    bool setMode(int feature, FeatureMode mode) override;
    bool getMode(int feature, FeatureMode* mode) override;
    bool setOnePush(int feature) override;

private:
    struct Feature
    {
        CameraSensor sensor;
        CameraOption option;
    };
    std::optional<Feature> manualOption(int feature);
    std::optional<Feature> automaticOption(int feature);
};

} // namespace dinrail

#endif // DINRAIL_YARPRGBDSENSORADAPTERS_H
