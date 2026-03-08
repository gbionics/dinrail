// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IPOSITIONDIRECT_H
#define DINRAIL_IPOSITIONDIRECT_H

namespace dinrail
{

/**
 * @ingroup dev_iface_motor
 *
 * Interface for a generic control board device implementing position control.
 * This interface is used to send high frequency streaming commands to the boards, the aim
 * is to reach low level control in firmware bypassing the trajectory generator.
 */
class IPositionDirect
{
public:
    /**
     * Destructor.
     */
    virtual ~IPositionDirect()
    {
    }

    /**
     * Get the number of controlled axes. This command asks the number of controlled
     * axes for the current physical interface.
     * @param ax pointer to storage for the return value
     * @return true/false on success/failure
     */
    virtual bool getAxes(int* ax) = 0;

    /** Set new position for a single axis.
     * @param j joint number
     * @param ref specifies the new ref point
     * @return true/false on success/failure
     */
    virtual bool setPosition(int j, double ref) = 0;

    /** Set new reference point for all axes.
     * @param n_joint how many joints this command is referring to
     * @param joints list of joints controlled. The size of this array is n_joints
     * @param refs array, new reference points, one value for each joint, the size is n_joints.
     *        The first value will be the new reference for the joint joints[0].
     *          for example:
     *          n_joint  3
     *          joints   0  2  4
     *          refs    10 30 40
     * @return true/false on success/failure
     */
    virtual bool setPositions(const int n_joint, const int* joints, const double* refs) = 0;

    /** Set new position for a set of axis.
     * @param refs specifies the new reference points
     * @return true/false on success/failure
     */
    virtual bool setPositions(const double* refs) = 0;

    /** Get the last position reference for the specified axis.
     *  This is the dual of setPositions and shall return only values sent using
     *  IPositionDirect interface.
     *  If other interfaces like IPositionControl are implemented by the device, this call
     *  must ignore their values, i.e. this call must never return a reference sent using
     *  IPositionControl::PositionMove.
     * @param joint joint number
     * @param ref last reference sent using setPosition(s) functions
     * @return true/false on success/failure
     */
    virtual bool getRefPosition(const int joint, double* ref) = 0;

    /** Get the last position reference for all axes.
     *  This is the dual of setPositions and shall return only values sent using
     *  IPositionDirect interface.
     *  If other interfaces like IPositionControl are implemented by the device, this call
     *  must ignore their values, i.e. this call must never return a reference sent using
     *  IPositionControl::PositionMove.
     * @param refs array containing last reference sent using setPosition(s) functions
     * @return true/false on success/failure
     */
    virtual bool getRefPositions(double* refs) = 0;

    /** Get the last position reference for the specified group of axes.
     *  This is the dual of setPositions and shall return only values sent using
     *  IPositionDirect interface.
     *  If other interfaces like IPositionControl are implemented by the device, this call
     *  must ignore their values, i.e. this call must never return a reference sent using
     *  IPositionControl::PositionMove.
     * @param n_joint how many joints this command is referring to
     * @param joints list of joints controlled
     * @param refs array containing last reference sent using setPosition(s) functions
     * @return true/false on success/failure
     */
    virtual bool getRefPositions(const int n_joint, const int* joints, double* refs) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IPOSITIONDIRECT_H
