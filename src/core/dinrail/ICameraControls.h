// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_ICAMERACONTROLS_H
#define DINRAIL_ICAMERACONTROLS_H

#include <dinrail/Status.h>

#include <optional>
#include <string_view>

namespace dinrail
{

/**
 * @brief Imaging module of an RGB-D camera that an option refers to.
 */
enum class CameraSensor
{
    Rgb, ///< The color camera.
    Depth, ///< The depth (stereo) module.
};

/**
 * @brief Camera options, expressed in the native units of the device.
 *
 * Boolean options (AutoExposure, AutoWhiteBalance, EmitterEnabled) use 0 for
 * false and 1 for true.
 */
enum class CameraOption
{
    AutoExposure,
    Exposure,
    Gain,
    AutoWhiteBalance,
    WhiteBalance,
    Brightness,
    Contrast,
    Gamma,
    Hue,
    Saturation,
    Sharpness,
    BacklightCompensation,
    PowerLineFrequency,
    EmitterEnabled,
    LaserPower,
};

/**
 * @brief snake_case name of the option, for example "auto_exposure".
 */
std::string_view toString(CameraOption option) noexcept;

/**
 * @brief Inverse of toString(CameraOption), std::nullopt for unknown names.
 */
std::optional<CameraOption> cameraOptionFromString(std::string_view name) noexcept;

struct CameraOptionRange
{
    double min{0.0};
    double max{0.0};
    double step{0.0};
    double defaultValue{0.0};
};

/**
 * @brief Interface to read and change the imaging options of a camera.
 *
 * Setting a manual exposure or gain disables the automatic exposure, and
 * setting a manual white balance disables the automatic white balance, as
 * otherwise the manual value would be ignored by the camera.
 */
class ICameraControls
{
public:
    virtual ~ICameraControls();

    virtual bool supportsOption(CameraSensor sensor, CameraOption option) = 0;
    virtual Status
    getOptionRange(CameraSensor sensor, CameraOption option, CameraOptionRange& range) = 0;
    virtual Status getOption(CameraSensor sensor, CameraOption option, double& value) = 0;
    virtual Status setOption(CameraSensor sensor, CameraOption option, double value) = 0;
};

} // namespace dinrail

#endif // DINRAIL_ICAMERACONTROLS_H
