// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_CAMERATYPES_H
#define DINRAIL_CAMERATYPES_H

#include <dinrail/IPreciselyTimed.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace dinrail
{

/**
 * @brief Pixel layout of an Image.
 */
enum class PixelFormat
{
    Unknown,
    Rgb8, ///< 3 bytes per pixel, R G B.
    Bgr8, ///< 3 bytes per pixel, B G R.
    Mono8, ///< 1 byte per pixel.
    Mono16, ///< 2 bytes per pixel, native endianness.
};

/**
 * @brief Number of bytes used by a single pixel of the given format, 0 for Unknown.
 */
std::size_t bytesPerPixel(PixelFormat format) noexcept;

/**
 * @brief Generic 2D image, stored row-major without padding between rows.
 *
 * The size of `data` is always `width * height * bytesPerPixel(format)`.
 */
struct Image
{
    std::uint32_t width{0};
    std::uint32_t height{0};
    PixelFormat format{PixelFormat::Unknown};
    std::vector<std::uint8_t> data;
    /// Acquisition time and sequence number. Images acquired together (for example
    /// the color and depth images returned by IRGBDSensor::getImages) share the same
    /// sequence number.
    Stamp stamp;
};

/**
 * @brief Depth image, stored row-major without padding between rows.
 *
 * Each element is the distance along the optical axis in meters. A value of 0
 * means that no valid measurement is available for that pixel.
 */
struct DepthImage
{
    std::uint32_t width{0};
    std::uint32_t height{0};
    std::vector<float> data;
    Stamp stamp;
};

/**
 * @brief Lens distortion model of a camera.
 */
enum class DistortionModel
{
    None,
    BrownConrady, ///< coefficients are k1, k2, p1, p2, k3.
    ModifiedBrownConrady, ///< Brown-Conrady with tangential terms computed on radially distorted
                          ///< points.
    InverseBrownConrady, ///< Brown-Conrady mapping from distorted to undistorted points.
    FTheta, ///< Equidistant fisheye model, coefficients[0] is the field of view parameter.
    KannalaBrandt4, ///< Fisheye model, coefficients are k1, k2, k3, k4.
};

std::string_view toString(DistortionModel model) noexcept;

/**
 * @brief Pinhole camera intrinsic parameters, in pixels.
 *
 * The principal point is expressed in the coordinate system in which the
 * center of the top-left pixel is (0, 0).
 */
struct CameraIntrinsics
{
    std::uint32_t width{0};
    std::uint32_t height{0};
    double focalLengthX{0.0};
    double focalLengthY{0.0};
    double principalPointX{0.0};
    double principalPointY{0.0};
    DistortionModel distortionModel{DistortionModel::None};
    std::array<double, 5> distortionCoefficients{};
};

/**
 * @brief Rigid transformation between two camera optical frames.
 *
 * A point `p_source` expressed in the source frame is expressed in the target
 * frame as `p_target = rotation * p_source + translation`. `rotation` is stored
 * row-major and `translation` is expressed in meters.
 */
struct CameraExtrinsics
{
    std::array<double, 9> rotation{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    std::array<double, 3> translation{0.0, 0.0, 0.0};
};

} // namespace dinrail

#endif // DINRAIL_CAMERATYPES_H
