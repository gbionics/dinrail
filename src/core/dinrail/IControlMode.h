// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_ICONTROLMODE_H
#define DINRAIL_ICONTROLMODE_H

#include <dinrail/Vocab.h>

namespace dinrail
{

// Control mode vocabulary values - compatible with YARP
// Values - Read / Write
constexpr vocab32_t VOCAB_CM_IDLE = createVocab32('i', 'd', 'l');
constexpr vocab32_t VOCAB_CM_TORQUE = createVocab32('t', 'o', 'r', 'q');
constexpr vocab32_t VOCAB_CM_POSITION = createVocab32('p', 'o', 's');
constexpr vocab32_t VOCAB_CM_POSITION_DIRECT = createVocab32('p', 'o', 's', 'd');
constexpr vocab32_t VOCAB_CM_VELOCITY = createVocab32('v', 'e', 'l');
constexpr vocab32_t VOCAB_CM_CURRENT = createVocab32('i', 'c', 'u', 'r');
constexpr vocab32_t VOCAB_CM_PWM = createVocab32('i', 'p', 'w', 'm');
constexpr vocab32_t VOCAB_CM_MIXED = createVocab32('m', 'i', 'x');

// Write only (only from high level toward the joint)
constexpr vocab32_t VOCAB_CM_FORCE_IDLE = createVocab32('f', 'i', 'd', 'l');

// Read only (imposed by the board on special events)
constexpr vocab32_t VOCAB_CM_HW_FAULT = createVocab32('h', 'w', 'f', 'a');
constexpr vocab32_t VOCAB_CM_CALIBRATING = createVocab32('c', 'a', 'l'); // the joint is calibrating
constexpr vocab32_t VOCAB_CM_CALIB_DONE = createVocab32('c', 'a', 'l', 'd'); // calibration
                                                                             // successfully
                                                                             // completed
constexpr vocab32_t VOCAB_CM_NOT_CONFIGURED = createVocab32('c', 'f', 'g', 'n'); // missing initial
                                                                                 // configuration
                                                                                 // (default value
                                                                                 // at start-up)
constexpr vocab32_t VOCAB_CM_CONFIGURED = createVocab32('c', 'f', 'g', 'y'); // initial
                                                                             // configuration
                                                                             // completed, if any

// Read only (cannot be set from user)
constexpr vocab32_t VOCAB_CM_UNKNOWN = createVocab32('u', 'n', 'k', 'w');

/**
 * @ingroup dev_iface_motor
 *
 * Interface for setting control mode in control board.
 */
class IControlMode
{
public:
    virtual ~IControlMode()
    {
    }

    /**
     * Get the current control mode.
     * @param j joint number
     * @param mode a vocab of the current control mode for joint j.
     * @return: true/false success failure.
     */
    virtual bool getControlMode(int j, int* mode) = 0;

    /**
     * Get the current control mode (multiple joints).
     * @param modes a vector containing vocabs for the current control modes of the joints.
     * @return: true/false success failure.
     */
    virtual bool getControlModes(int* modes) = 0;

    /**
     * Get the current control mode for a subset of axes.
     * @param n_joints how many joints this command is referring to
     * @param joints list of joint numbers, the size of this array is n_joints
     * @param modes array containing the new controlmodes, one value for each joint, the size is
     * n_joints. The first value will be the new reference for the joint joints[0]. for example:
     *          n_joint  3
     *          joints   0  2  4
     *          modes    VOCAB_CM_POSITION VOCAB_CM_VELOCITY VOCAB_CM_POSITION
     * @return true/false success failure.
     */
    virtual bool getControlModes(const int n_joint, const int* joints, int* modes) = 0;

    /**
     * Set the current control mode.
     * @param j: joint number
     * @param mode: a vocab of the desired control mode for joint j.
     * @return true if the new controlMode was successfully set, false if the message was not
     * received or the joint was unable to switch to the desired controlMode (e.g. the joint is on a
     * fault condition or the desired mode is not implemented).
     */
    virtual bool setControlMode(const int j, const int mode) = 0;

    /**
     * Set the current control mode for a subset of axes.
     * @param n_joints how many joints this command is referring to
     * @param joints list of joint numbers, the size of this array is n_joints
     * @param modes array containing the new controlmodes, one value for each joint, the size is
     * n_joints. The first value will be the new reference for the joint joints[0]. for example:
     *          n_joint  3
     *          joints   0  2  4
     *          modes    VOCAB_CM_POSITION VOCAB_CM_VELOCITY VOCAB_CM_POSITION
     * @return true if the new controlMode was successfully set, false if the message was not
     * received or the joint was unable to switch to the desired controlMode (e.g. the joint is on a
     * fault condition or the desired mode is not implemented).
     */
    virtual bool setControlModes(const int n_joint, const int* joints, int* modes) = 0;

    /**
     * Set the current control mode (multiple joints).
     * @param modes: a vector containing vocabs for the desired control modes of the joints.
     * @return true if the new controlMode was successfully set, false if the message was not
     * received or the joint was unable to switch to the desired controlMode (e.g. the joint is on a
     * fault condition or the desired mode is not implemented).
     */
    virtual bool setControlModes(int* modes) = 0;
};

} // namespace dinrail

#endif // DINRAIL_ICONTROLMODE_H
