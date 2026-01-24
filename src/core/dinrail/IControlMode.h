// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_ICONTROLMODE_H
#define DINRAIL_ICONTROLMODE_H

namespace dinrail {

// Control mode vocabulary values
constexpr int VOCAB_CM_IDLE            = 0;
constexpr int VOCAB_CM_TORQUE          = 1;
constexpr int VOCAB_CM_POSITION        = 2;
constexpr int VOCAB_CM_POSITION_DIRECT = 3;
constexpr int VOCAB_CM_VELOCITY        = 4;
constexpr int VOCAB_CM_CURRENT         = 5;
constexpr int VOCAB_CM_PWM             = 6;
constexpr int VOCAB_CM_MIXED           = 7;
constexpr int VOCAB_CM_FORCE_IDLE      = 8;
constexpr int VOCAB_CM_HW_FAULT        = 9;
constexpr int VOCAB_CM_CALIBRATING     = 10;
constexpr int VOCAB_CM_CALIB_DONE      = 11;
constexpr int VOCAB_CM_NOT_CONFIGURED  = 12;
constexpr int VOCAB_CM_CONFIGURED      = 13;
constexpr int VOCAB_CM_UNKNOWN         = 14;

/**
 * @ingroup dev_iface_motor
 *
 * Interface for setting control mode in control board.
 */
class IControlMode
{
public:
    virtual ~IControlMode() {}

    /**
     * Get the current control mode.
     * @param j joint number
     * @param mode a vocab of the current control mode for joint j.
     * @return: true/false success failure.
     */
    virtual bool getControlMode(int j, int *mode) = 0;

    /**
     * Get the current control mode (multiple joints).
     * @param modes a vector containing vocabs for the current control modes of the joints.
     * @return: true/false success failure.
     */
    virtual bool getControlModes(int *modes) = 0;

    /**
     * Get the current control mode for a subset of axes.
     * @param n_joints how many joints this command is referring to
     * @param joints list of joint numbers, the size of this array is n_joints
     * @param modes array containing the new controlmodes, one value for each joint, the size is n_joints.
     *          The first value will be the new reference for the joint joints[0].
     *          for example:
     *          n_joint  3
     *          joints   0  2  4
     *          modes    VOCAB_CM_POSITION VOCAB_CM_VELOCITY VOCAB_CM_POSITION
     * @return true/false success failure.
     */
    virtual bool getControlModes(const int n_joint, const int *joints, int *modes) = 0;

    /**
     * Set the current control mode.
     * @param j: joint number
     * @param mode: a vocab of the desired control mode for joint j.
     * @return true if the new controlMode was successfully set, false if the message was not received or
     *         the joint was unable to switch to the desired controlMode
     *         (e.g. the joint is on a fault condition or the desired mode is not implemented).
     */
    virtual bool setControlMode(const int j, const int mode) = 0;

    /**
     * Set the current control mode for a subset of axes.
     * @param n_joints how many joints this command is referring to
     * @param joints list of joint numbers, the size of this array is n_joints
     * @param modes array containing the new controlmodes, one value for each joint, the size is n_joints.
     *          The first value will be the new reference for the joint joints[0].
     *          for example:
     *          n_joint  3
     *          joints   0  2  4
     *          modes    VOCAB_CM_POSITION VOCAB_CM_VELOCITY VOCAB_CM_POSITION
     * @return true if the new controlMode was successfully set, false if the message was not received or
     *         the joint was unable to switch to the desired controlMode
     *         (e.g. the joint is on a fault condition or the desired mode is not implemented).
     */
    virtual bool setControlModes(const int n_joint, const int *joints, int *modes) = 0;

    /**
     * Set the current control mode (multiple joints).
     * @param modes: a vector containing vocabs for the desired control modes of the joints.
     * @return true if the new controlMode was successfully set, false if the message was not received or
     *         the joint was unable to switch to the desired controlMode
     *         (e.g. the joint is on a fault condition or the desired mode is not implemented).
     */
    virtual bool setControlModes(int *modes) = 0;
};

} // namespace dinrail

#endif // DINRAIL_ICONTROLMODE_H
