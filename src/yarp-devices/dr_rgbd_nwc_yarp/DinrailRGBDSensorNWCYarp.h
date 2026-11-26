// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DINRAILRGBDSENSORNWCYARP_H
#define DINRAIL_DINRAILRGBDSENSORNWCYARP_H

#include <dinrail/IRGBDSensor.h>

#include <yarp/dev/DeviceDriver.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Port.h>
#include <yarp/os/TypedReaderCallback.h>
#include <yarp/sig/Image.h>

#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

/**
 * @brief Network wrapper client of an RGBD sensor, exposing dinrail::IRGBDSensor.
 *
 * The device connects to the ports of dr_rgbd_nws_yarp, or of the YARP rgbdSensor_nws_yarp, as
 * they use the same protocol. The images are received by port callbacks, which store the most
 * recent ones: the getters never block, and return the same images until new ones are received.
 * The intrinsic and extrinsic parameters and the clip planes are read through the RPC port when
 * the device is opened, so that the getters do not communicate with the server.
 *
 * The color stream is optional: if @c remote_image_port is not set only the depth is received,
 * and the getters of the color image return dinrail::StatusCode::FailedPrecondition.
 *
 * See README.md for the parameters.
 */
class DinrailRGBDSensorNWCYarp : public yarp::dev::DeviceDriver, public dinrail::IRGBDSensor
{
public:
    DinrailRGBDSensorNWCYarp();
    ~DinrailRGBDSensorNWCYarp() override;

    // yarp::dev::DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;

    // dinrail::IRGBDSensor
    dinrail::RGBDSensorStatus getSensorStatus() override;
    std::string getLastErrorMessage() override;
    dinrail::Status getRgbIntrinsics(dinrail::CameraIntrinsics& intrinsics) override;
    dinrail::Status getDepthIntrinsics(dinrail::CameraIntrinsics& intrinsics) override;
    dinrail::Status getDepthToRgbExtrinsics(dinrail::CameraExtrinsics& extrinsics) override;
    dinrail::Status getDepthClipPlanes(double& nearPlane, double& farPlane) override;
    dinrail::Status getRgbImage(dinrail::Image& rgbImage) override;
    dinrail::Status getDepthImage(dinrail::DepthImage& depthImage) override;
    dinrail::Status getImages(dinrail::Image& rgbImage, dinrail::DepthImage& depthImage) override;
    dinrail::Status getImagesStamp(dinrail::Stamp& stamp) override;

private:
    using Clock = std::chrono::steady_clock;

    class RgbReader : public yarp::os::TypedReaderCallback<yarp::sig::FlexImage>
    {
    public:
        explicit RgbReader(DinrailRGBDSensorNWCYarp& owner);
        using yarp::os::TypedReaderCallback<yarp::sig::FlexImage>::onRead;
        void onRead(yarp::sig::FlexImage& image) override;

    private:
        DinrailRGBDSensorNWCYarp& m_owner;
    };

    class DepthReader
        : public yarp::os::TypedReaderCallback<yarp::sig::ImageOf<yarp::sig::PixelFloat>>
    {
    public:
        explicit DepthReader(DinrailRGBDSensorNWCYarp& owner);
        using yarp::os::TypedReaderCallback<yarp::sig::ImageOf<yarp::sig::PixelFloat>>::onRead;
        void onRead(yarp::sig::ImageOf<yarp::sig::PixelFloat>& image) override;

    private:
        DinrailRGBDSensorNWCYarp& m_owner;
    };

    bool connectServer(const yarp::os::Searchable& config);
    bool readParameters();
    void closePorts();
    /** Must be called with m_imagesMutex locked. */
    void updatePair();

    bool m_hasRgb{false};
    std::chrono::nanoseconds m_frameTimeout{std::chrono::seconds(1)};
    std::string m_remoteRpcPort;

    RgbReader m_rgbReader;
    DepthReader m_depthReader;
    yarp::os::BufferedPort<yarp::sig::FlexImage> m_rgbPort;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelFloat>> m_depthPort;
    yarp::os::Port m_rpcPort;

    // Parameters read when the device is opened.
    std::optional<dinrail::CameraIntrinsics> m_rgbIntrinsics;
    std::optional<dinrail::CameraIntrinsics> m_depthIntrinsics;
    std::optional<dinrail::CameraExtrinsics> m_extrinsics;
    std::optional<std::pair<double, double>> m_clipPlanes;

    // Latest images, written by the port callbacks. Readers only copy the shared pointers under
    // the lock, so they never wait for the conversion of a received image.
    std::mutex m_imagesMutex;
    std::shared_ptr<const dinrail::Image> m_latestRgb;
    std::shared_ptr<const dinrail::DepthImage> m_latestDepth;
    // Latest color and depth images with the same sequence number, returned by getImages().
    std::shared_ptr<const dinrail::Image> m_pairedRgb;
    std::shared_ptr<const dinrail::DepthImage> m_pairedDepth;
    Clock::time_point m_lastDepthArrival;
};

#endif // DINRAIL_DINRAILRGBDSENSORNWCYARP_H
