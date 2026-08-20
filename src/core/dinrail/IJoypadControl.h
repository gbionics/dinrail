// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: LicenseRef-GenerativeBionics-AllRightsReserved

#ifndef DINRAIL_IJOYPAD_CONTROL_H
#define DINRAIL_IJOYPAD_CONTROL_H

namespace dinrail
{

/**
 * Device-level events reported by a joypad backend.
 *
 * The initial set mirrors the SDL joystick device events
 * (https://wiki.libsdl.org/SDL3/SDL_JoyDeviceEvent) and can be extended as
 * needed.
 */
enum class JoypadDeviceEvent
{
    /// No event available, or device events are not supported by the backend.
    NoEvent,
    /// The device was connected (SDL_EVENT_JOYSTICK_ADDED).
    Connected,
    /// The device was disconnected (SDL_EVENT_JOYSTICK_REMOVED).
    Disconnected,
    /// The device finished updating its state (SDL_EVENT_JOYSTICK_UPDATE_COMPLETE).
    UpdateComplete,
};

/**
 * Interface for a joystick-like input device.
 *
 * It covers both raw input reading and backend-specific reconnect support.
 *
 * Axes are normalised to [-1.0, 1.0]. Button values are 0.0 (released) or
 * 1.0 (pressed). Hat values follow the bitmask convention of the underlying
 * backend.
 */
class IJoypadControl
{
public:
    virtual ~IJoypadControl();

    // --- Input reading ---

    /**
     * @brief Get the number of controlled axes.
     * @param count Output variable receiving the number of axes.
     * @return true on success, false otherwise.
     */
    virtual bool getAxisCount(unsigned int& count) = 0;

    /**
     * @brief Get the number of controlled buttons.
     * @param count Output variable receiving the number of buttons.
     * @return true on success, false otherwise.
     */
    virtual bool getButtonCount(unsigned int& count) = 0;

    /**
     * @brief Get the number of controlled hats.
     * @param count Output variable receiving the number of hats.
     * @return true on success, false otherwise.
     */
    virtual bool getHatCount(unsigned int& count) = 0;

    /**
     * @brief Get the state of an axis.
     * @param axis_id Id of the axis to read.
     * @param value Output variable receiving the axis value.
     * @return true on success, false otherwise.
     */
    virtual bool getAxis(unsigned int axis_id, double& value) = 0;

    /**
     * @brief Get the state of a button.
     * @param button_id Id of the button to read.
     * @param value Output variable receiving the button value.
     * @return true on success, false otherwise.
     */
    virtual bool getButton(unsigned int button_id, float& value) = 0;

    /**
     * @brief Get the state of a hat.
     * @param hat_id Id of the hat to read.
     * @param value Output variable receiving the hat value.
     * @return true on success, false otherwise.
     */
    virtual bool getHat(unsigned int hat_id, unsigned char& value) = 0;

    // --- Backend reconnect support ---

    /**
     * Reset the backend so the device is ready to be used again after a
     * disconnect.
     *
     * The exact behavior is backend-dependent.
     * Example (SDL): call SDL_QuitSubSystem(SDL_INIT_JOYSTICK), then
     * SDL_InitSubSystem(SDL_INIT_JOYSTICK) to perform a fresh device
     * enumeration.
     */
    virtual bool reconnect() = 0;

    /**
     * Get the most recent device-level event reported by the backend.
     *
     * This method does not mutate any internal state, so it is safe to call
     * from concurrent applications reading the same interface. It reports the
     * last known event rather than consuming it from a queue. After a
     * successful reconnect(), the reported event becomes
     * JoypadDeviceEvent::Connected.
     *
     * Backends that do not support device events return
     * JoypadDeviceEvent::NoEvent.
     *
     * @param event Output variable receiving the last device event.
     * @return true on success, false otherwise.
     */
    virtual bool getLastEvent(JoypadDeviceEvent& event) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IJOYPAD_CONTROL_H
