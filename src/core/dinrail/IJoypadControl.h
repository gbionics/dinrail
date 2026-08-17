// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: LicenseRef-GenerativeBionics-AllRightsReserved

#ifndef DINRAIL_IJOYPAD_CONTROL_H
#define DINRAIL_IJOYPAD_CONTROL_H

namespace dinrail
{

/**
 * Interface for an SDL-backed joystick device.
 *
 * Covers both raw input reading and SDL subsystem lifecycle management needed
 * for robust reconnection workflows.
 *
 * Axes are normalised to [-1.0, 1.0]. Button values are 0.0 (released) or
 * 1.0 (pressed). Hat values follow the SDL bitmask convention (bit 0 = up,
 * bit 1 = right, bit 2 = down, bit 3 = left).
 */
class IJoypadControl
{
public:
    virtual ~IJoypadControl() = default;

    // --- Input reading ---

    virtual bool getAxisCount(unsigned int& count) = 0;
    virtual bool getButtonCount(unsigned int& count) = 0;
    virtual bool getHatCount(unsigned int& count) = 0;

    virtual bool getAxis(unsigned int axis_id, double& value) = 0;
    virtual bool getButton(unsigned int button_id, float& value) = 0;
    virtual bool getHat(unsigned int hat_id, unsigned char& value) = 0;

    // --- SDL subsystem lifecycle ---

    /**
     * Quit the SDL joystick subsystem and immediately re-initialise it so
     * that the next open() call performs a fresh hardware enumeration.
     * Call this before re-opening the device after a disconnect.
     */
    virtual bool reinitJoystickSubsystem() = 0;

    /**
     * Return true if an SDL_JOYDEVICEREMOVED event is pending in the SDL
     * event queue (and drain it).
     */
    virtual bool isDeviceRemoved() = 0;
};

} // namespace dinrail

#endif // DINRAIL_IJOYPAD_CONTROL_H
