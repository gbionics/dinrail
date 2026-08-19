// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: LicenseRef-GenerativeBionics-AllRightsReserved

#ifndef DINRAIL_IJOYPAD_CONTROL_H
#define DINRAIL_IJOYPAD_CONTROL_H

namespace dinrail
{

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
     * Prepare the backend for a reconnect attempt.
     *
     * Call this before re-opening the device after a disconnect.
     *
     * The exact behavior is backend-dependent.
     * Example (SDL): call SDL_QuitSubSystem(SDL_INIT_JOYSTICK), then
     * SDL_InitSubSystem(SDL_INIT_JOYSTICK), so the next open() performs a
     * fresh device enumeration.
     */
    virtual bool prepareForReconnect() = 0;

    /**
     * Consume one pending disconnect notification from the backend.
     *
     * Returns true only when a disconnect notification was present and
     * consumed.
     *
     * Example (SDL): check for SDL_JOYDEVICEREMOVED and remove exactly one
     * matching event from the queue.
     */
    virtual bool consumeDisconnectEvent() = 0;
};

} // namespace dinrail

#endif // DINRAIL_IJOYPAD_CONTROL_H
