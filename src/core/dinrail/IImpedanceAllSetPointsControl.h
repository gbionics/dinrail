// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IIMPEDANCEALLSETPOINTSCONTROL_H
#define DINRAIL_IIMPEDANCEALLSETPOINTSCONTROL_H

#include <string>

#include <dinrail/VectorProxy.h>

namespace dinrail
{

/**
 * @brief Interface to set all impedance control setpoints (position, velocity, torque, stiffness, damping) at once.
 *
 * This interface permits to set all impedance control setpoints for multiple axes in a single method call, 
 * ensuring that a set of parameters for impedance control is applied simultaneously.
 * 
 * This way of setting impedance control parameters is sometimes called "MIT mode control" or 
 * "MIT control", as it was popularized by the MIT Cheetah robot, see:
 *   * https://ieeexplore.ieee.org/abstract/document/8793865 
 *   * https://www.cubemars.com/ak-series-robotic-actuator-mit-mode-control.html
 *
 * This interface is meant to be used when the joint is controlled in `POSITION_DIRECT`,
 * even if also velocity setpoint and torque feedforward are used, to permit to control the
 * joint even using the IPositionDirect interface, while keeping the rest of values constant.
 *
 * The unit of the position, velocity, torque, stiffness and damping setpoints is the same as the one used by the:
 *  * position: IPositionControl,
 *  * velocity: IVelocityControl,
 *  * torque: ITorqueControl,
 *  * stiffness and damping: IImpedanceControl.
 * 
 */
class IImpedanceAllSetPointsControl
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~IImpedanceAllSetPointsControl() = default;

    /**
     * @brief Set all impedance control setpoints for a single joint.
     *
     * @param j Joint index.
     * @param pos Position setpoint.
     * @param vel Velocity setpoint.
     * @param torque Torque feedforward setpoint.
     * @param stiffness Stiffness setpoint.
     * @param damping Damping setpoint.
     * @return True on success, false otherwise.
     */
    virtual bool setSetPoint(int j, double pos, double vel, double torque, double stiffness, double damping)=0;
 
    /**
     * @brief Set all impedance control setpoints for a subset of joints.
     *
     * @param jointIndeces Joint indices to update.
     * @param pos Position setpoints, one for each joint in @p jointIndeces.
     * @param vel Velocity setpoints, one for each joint in @p jointIndeces.
     * @param torque Torque feedforward setpoints, one for each joint in @p jointIndeces.
     * @param stiffness Stiffness setpoints, one for each joint in @p jointIndeces.
     * @param damping Damping setpoints, one for each joint in @p jointIndeces.
     * @return True on success, false otherwise.
     */
    virtual bool setSetPoints(const VectorProxy<const int>::Ref jointIndeces,
                              const VectorProxy<const double>::Ref pos,
                              const VectorProxy<const double>::Ref vel,
                              const VectorProxy<const double>::Ref torque,
                              const VectorProxy<const double>::Ref stiffness,
                              const VectorProxy<const double>::Ref damping)=0;
 
    /**
     * @brief Set all impedance control setpoints for all controlled joints.
     *
     * @param pos Position setpoints for all controlled joints.
     * @param vel Velocity setpoints for all controlled joints.
     * @param torque Torque feedforward setpoints for all controlled joints.
     * @param stiffness Stiffness setpoints for all controlled joints.
     * @param damping Damping setpoints for all controlled joints.
     * @return True on success, false otherwise.
     */
    virtual bool setSetPoints(const VectorProxy<const double>::Ref pos,
                              const VectorProxy<const double>::Ref vel,
                              const VectorProxy<const double>::Ref torque,
                              const VectorProxy<const double>::Ref stiffness,
                              const VectorProxy<const double>::Ref damping)=0;

    /**
     * @brief Get all impedance control setpoints for a single joint.
     *
     * @warning depending on the implementation, the get method may be 
     *          blocking as it may request the setpoints over RPC, before
     *          using it in the high frequency loop check the actual underlying
     *          device.
     * 
     * @param j Joint index.
     * @param pos Position setpoint.
     * @param vel Velocity setpoint.
     * @param torque Torque feedforward setpoint.
     * @param stiffness Stiffness setpoint.
     * @param damping Damping setpoint.
     * @return True on success, false otherwise.
     */
    virtual bool getSetPoint(int j,
                             double& pos,
                             double& vel,
                             double& torque,
                             double& stiffness,
                             double& damping)=0;

    /**
     * @brief Get all impedance control setpoints for a subset of joints.
     *
     * @warning depending on the implementation, the get method may be 
     *          blocking as it may request the setpoints over RPC, before
     *          using it in the high frequency loop check the actual underlying
     *          device.
     *
     * @param jointIndeces Joint indices to query.
     * @param pos Position setpoints, one for each joint in @p jointIndeces.
     * @param vel Velocity setpoints, one for each joint in @p jointIndeces.
     * @param torque Torque feedforward setpoints, one for each joint in @p jointIndeces.
     * @param stiffness Stiffness setpoints, one for each joint in @p jointIndeces.
     * @param damping Damping setpoints, one for each joint in @p jointIndeces.
     * @return True on success, false otherwise.
     */
    virtual bool getSetPoints(const VectorProxy<const int>::Ref jointIndeces,
                              VectorProxy<double>::Ref pos,
                              VectorProxy<double>::Ref vel,
                              VectorProxy<double>::Ref torque,
                              VectorProxy<double>::Ref stiffness,
                              VectorProxy<double>::Ref damping)=0;

    /**
     * @brief Get all impedance control setpoints for all controlled joints.
     *
     * @warning depending on the implementation, the get method may be 
     *          blocking as it may request the setpoints over RPC, before
     *          using it in the high frequency loop check the actual underlying
     *          device.
     *
     * @param pos Position setpoints for all controlled joints.
     * @param vel Velocity setpoints for all controlled joints.
     * @param torque Torque feedforward setpoints for all controlled joints.
     * @param stiffness Stiffness setpoints for all controlled joints.
     * @param damping Damping setpoints for all controlled joints.
     * @return True on success, false otherwise.
     */
    virtual bool getSetPoints(VectorProxy<double>::Ref pos,
                              VectorProxy<double>::Ref vel,
                              VectorProxy<double>::Ref torque,
                              VectorProxy<double>::Ref stiffness,
                              VectorProxy<double>::Ref damping)=0;

};

} // namespace dinrail

#endif // DINRAIL_IIMPEDANCEALLSETPOINTSCONTROL_H
