// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IRGBDSENSOR_H
#define DINRAIL_IRGBDSENSOR_H

#include <dinrail/CameraTypes.h>

#include <dinrail/Status.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace dinrail
{

/**
 * @brief State of an RGB-D sensor.
 */
enum class RGBDSensorStatus
{
    NotReady, ///< The sensor is starting, no image has been received yet.
    Ok, ///< Images are being received.
    Timeout, ///< No image has been received for longer than the configured timeout.
    Disconnected, ///< The sensor has been disconnected.
    Error, ///< The sensor is in a generic error state, see getLastErrorMessage().
};

std::string_view toString(RGBDSensorStatus status) noexcept;

/**
 * @brief Interface of a sensor that provides synchronized color and depth images.
 *
 * Image getters never block: they return the most recent images acquired by
 * the sensor, which are updated in the background as new images arrive. If no
 * new image arrived since the previous call, the same images are returned
 * again: compare their stamp sequence numbers to detect new images. If no
 * image has been acquired yet, StatusCode::Unavailable is returned.
 *
 * The intrinsic parameters and the image sizes always describe the images
 * actually returned by the getters, after any processing (such as alignment
 * or rotation) performed by the device.
 *
 * A sensor can be configured to stream only depth images. In that case the
 * methods that involve the color images (getRgbIntrinsics(),
 * getDepthToRgbExtrinsics(), getRgbImage() and getImages()) return
 * StatusCode::FailedPrecondition.
 */
class IRGBDSensor
{
public:
    virtual ~IRGBDSensor();

    virtual RGBDSensorStatus getSensorStatus() = 0;

    /**
     * @brief Human readable description of the last error, empty if no error occurred.
     */
    virtual std::string getLastErrorMessage() = 0;

    virtual Status getRgbIntrinsics(CameraIntrinsics& intrinsics) = 0;
    virtual Status getDepthIntrinsics(CameraIntrinsics& intrinsics) = 0;

    /**
     * @brief Transformation from the depth optical frame to the color optical frame.
     */
    virtual Status getDepthToRgbExtrinsics(CameraExtrinsics& extrinsics) = 0;

    /**
     * @brief Range of depth values returned by the sensor, in meters.
     *
     * Depth values outside [nearPlane, farPlane] are reported as 0. If no
     * clipping is performed, nearPlane is 0 and farPlane is +infinity.
     */
    virtual Status getDepthClipPlanes(double& nearPlane, double& farPlane) = 0;

    virtual Status getRgbImage(Image& rgbImage) = 0;
    virtual Status getDepthImage(DepthImage& depthImage) = 0;

    /**
     * @brief Get the most recent color and depth images acquired together.
     *
     * Both images have the same stamp sequence number.
     */
    virtual Status getImages(Image& rgbImage, DepthImage& depthImage) = 0;

    /**
     * @brief Get the stamp of the most recent depth image, which is also the stamp of the images
     * returned by getImages(), without copying them.
     *
     * It never waits, so it can be polled at a high rate to detect new images. The default
     * implementation returns StatusCode::Unimplemented.
     */
    virtual Status getImagesStamp(Stamp& stamp);
};

/**
 * @brief Simulation counterpart of IRGBDSensor, used to implement fake devices.
 */
class IRGBDSensorSimulation
{
public:
    virtual ~IRGBDSensorSimulation();

    virtual Status setImages(const Image& rgbImage, const DepthImage& depthImage) = 0;
    virtual Status setSensorStatus(RGBDSensorStatus status) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IRGBDSENSOR_H
