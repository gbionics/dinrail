// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DRCAMERAFAKE_H
#define DINRAIL_DRCAMERAFAKE_H
#include <array>
#include <dinrail/IDevice.h>
#include <dinrail/IFrameGrabberControls.h>
#include <dinrail/IFrameGrabberImageRaw.h>
#include <dinrail/IRGBDSensor.h>
#include <mutex>
namespace dinrail
{
/** A simulator-driven camera. mode=framegrabber disables depth acquisition. */
class DrCameraFake final : public IDevice,
                           public IFrameGrabberControls,
                           public IFrameGrabberControlsSimulation,
                           public IFrameGrabberImageRaw,
                           public IFrameGrabberImageRawSimulation,
                           public IRGBDSensor,
                           public IRGBDSensorSimulation,
                           public IPreciselyTimed,
                           public IPreciselyTimedSimulation
{
public:
    bool open(const Parameters& config) override;
    bool close() override;
    Stamp getLastInputStamp() override;
    void setLastInputStamp(const Stamp& stamp) override;
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
    int height() const override;
    int width() const override;
    Status getImage(cv::Mat& image) override;
    Status setImage(const cv::Mat& image) override;
    Status setRgbImage(const cv::Mat& image, const Stamp& stamp) override;
    Status setDepthImage(const cv::Mat& image, const Stamp& stamp) override;
    Status setImages(const cv::Mat& rgb,
                     const cv::Mat& depth,
                     const Stamp& rgbStamp,
                     const Stamp& depthStamp) override;
    Status setRgbIntrinsicParam(const Parameters& intrinsic) override;
    Status setDepthIntrinsicParam(const Parameters& intrinsic) override;
    Status setExtrinsicParam(const cv::Mat& extrinsic) override;
    Status setSensorStatus(RGBDSensorStatus status) override;
    Status setLastErrorMsg(const std::string& message, const Stamp& stamp) override;
    Status setCameraDescription(const CameraDescriptor& camera) override;
    Status setFeatureCapabilities(int feature,
                                  bool available,
                                  bool onOff,
                                  bool automatic,
                                  bool manual,
                                  bool onePush) override;

private:
    struct Feature
    {
        bool available{true}, onOff{true}, automatic{true}, manual{true}, onePush{true},
            active{true};
        double value1{0.5}, value2{0.5};
        FeatureMode mode{FeatureMode::Manual};
    };
    // Called with m_mutex held; the diagnostic pattern is generated on acquisition.
    void updateTestPattern();
    Status ready(bool depth = false) const;
    Status featureReady(int feature) const;
    mutable std::mutex m_mutex;
    bool m_opened{false}, m_rgbd{true}, m_testPattern{false};
    cv::Mat m_raw, m_rgb, m_depth, m_extrinsic, m_pattern;
    Stamp m_rgbStamp, m_depthStamp, m_lastStamp, m_errorStamp;
    Parameters m_rgbIntrinsic, m_depthIntrinsic;
    double m_rgbHFov{60}, m_rgbVFov{45}, m_depthHFov{60}, m_depthVFov{45};
    double m_accuracy{0.001}, m_near{0.1}, m_far{10};
    bool m_rgbMirror{false}, m_depthMirror{false};
    CameraDescriptor m_description;
    RGBDSensorStatus m_status{RGBDSensorStatus::NotReady};
    std::string m_error;
    std::array<Feature, static_cast<int>(CameraFeature::Count)> m_features;
};
} // namespace dinrail
#endif
