// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/CameraTypes.h>
#include <dinrail/ICameraControls.h>
#include <dinrail/IRGBDSensor.h>

#include <array>
#include <utility>

namespace dinrail
{

namespace
{

constexpr std::array<std::pair<CameraOption, std::string_view>, 15> cameraOptionNames{{
    {CameraOption::AutoExposure, "auto_exposure"},
    {CameraOption::Exposure, "exposure"},
    {CameraOption::Gain, "gain"},
    {CameraOption::AutoWhiteBalance, "auto_white_balance"},
    {CameraOption::WhiteBalance, "white_balance"},
    {CameraOption::Brightness, "brightness"},
    {CameraOption::Contrast, "contrast"},
    {CameraOption::Gamma, "gamma"},
    {CameraOption::Hue, "hue"},
    {CameraOption::Saturation, "saturation"},
    {CameraOption::Sharpness, "sharpness"},
    {CameraOption::BacklightCompensation, "backlight_compensation"},
    {CameraOption::PowerLineFrequency, "power_line_frequency"},
    {CameraOption::EmitterEnabled, "emitter_enabled"},
    {CameraOption::LaserPower, "laser_power"},
}};

} // namespace

std::size_t bytesPerPixel(PixelFormat format) noexcept
{
    switch (format)
    {
    case PixelFormat::Rgb8:
    case PixelFormat::Bgr8:
        return 3;
    case PixelFormat::Mono8:
        return 1;
    case PixelFormat::Mono16:
        return 2;
    case PixelFormat::Unknown:
        break;
    }
    return 0;
}

std::string_view toString(DistortionModel model) noexcept
{
    switch (model)
    {
    case DistortionModel::None:
        return "none";
    case DistortionModel::BrownConrady:
        return "brown_conrady";
    case DistortionModel::ModifiedBrownConrady:
        return "modified_brown_conrady";
    case DistortionModel::InverseBrownConrady:
        return "inverse_brown_conrady";
    case DistortionModel::FTheta:
        return "ftheta";
    case DistortionModel::KannalaBrandt4:
        return "kannala_brandt4";
    }
    return "unknown";
}

std::string_view toString(RGBDSensorStatus status) noexcept
{
    switch (status)
    {
    case RGBDSensorStatus::NotReady:
        return "not_ready";
    case RGBDSensorStatus::Ok:
        return "ok";
    case RGBDSensorStatus::Timeout:
        return "timeout";
    case RGBDSensorStatus::Disconnected:
        return "disconnected";
    case RGBDSensorStatus::Error:
        return "error";
    }
    return "unknown";
}

std::string_view toString(CameraOption option) noexcept
{
    for (const auto& [value, name] : cameraOptionNames)
    {
        if (value == option)
        {
            return name;
        }
    }
    return "unknown";
}

std::optional<CameraOption> cameraOptionFromString(std::string_view name) noexcept
{
    for (const auto& [value, optionName] : cameraOptionNames)
    {
        if (optionName == name)
        {
            return value;
        }
    }
    return std::nullopt;
}

} // namespace dinrail
