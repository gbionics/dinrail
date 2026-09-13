// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DINRAILCONTROLBOARDINTERFACEADAPTERS_H
#define DINRAIL_DINRAILCONTROLBOARDINTERFACEADAPTERS_H

#include <dinrail/IAxisInfo.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/IJointFault.h>
#include <dinrail/IMotor.h>
#include <dinrail/IMotorEncoders.h>
#include <dinrail/InterfaceAdapter.h>
#include <span>
#include <yarp/dev/IAxisInfo.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IEncodersTimed.h>
#include <yarp/dev/IJointFault.h>
#include <yarp/dev/IMotor.h>
#include <yarp/dev/IMotorEncoders.h>

namespace dinrail
{
template <>
class InterfaceAdapter<yarp::dev::IAxisInfo, dinrail::IAxisInfo>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IAxisInfo, dinrail::IAxisInfo>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool getAxes(int* axes) override;
    bool getAxisName(int axis, std::string& name) override;
    bool getJointType(int axis, yarp::dev::JointTypeEnum& type) override;
}
;

template <>
class InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IEncodersTimed, dinrail::IEncoders>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool getAxes(int* count) override;
    bool getEncoder(int axis, double* value) override;
    bool getEncoderSpeed(int axis, double* value) override;
    bool getEncoderAcceleration(int axis, double* value) override;
    bool getEncoderTimed(int axis, double* value, double* timestamp) override;
    bool getEncoders(double* values) override;
    bool getEncoderSpeeds(double* values) override;
    bool getEncoderAccelerations(double* values) override;
    bool getEncodersTimed(double* values, double* timestamps) override;
    // Calibration and reset operations were deliberately dropped from dinrail's measurement
    // interface.
    bool resetEncoder(int) override;
    bool resetEncoders() override;
    bool setEncoder(int, double) override;
    bool setEncoders(const double*) override;
}
;

template <>
class InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool getNumberOfMotorEncoders(int* count) override;
    bool getMotorEncoder(int axis, double* value) override;
    bool getMotorEncoderSpeed(int axis, double* value) override;
    bool getMotorEncoderAcceleration(int axis, double* value) override;
    bool getMotorEncoderTimed(int axis, double* value, double* timestamp) override;
    bool getMotorEncoders(double* values) override;
    bool getMotorEncoderSpeeds(double* values) override;
    bool getMotorEncoderAccelerations(double* values) override;
    bool getMotorEncodersTimed(double* values, double* timestamps) override;
    // Calibration and reset operations were deliberately dropped from dinrail's measurement
    // interface.
    bool resetMotorEncoder(int) override;
    bool resetMotorEncoders() override;
    bool setMotorEncoder(int, double) override;
    bool setMotorEncoders(const double*) override;
    bool setMotorEncoderCountsPerRevolution(int, double) override;
    bool getMotorEncoderCountsPerRevolution(int axis, double* counts) override;
}
;

template <>
class InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IMotor, dinrail::IMotor>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool getNumberOfMotors(int* count) override;
    bool getTemperature(int motor, double* value) override;
    bool getTemperatureLimit(int motor, double* value) override;
    bool setTemperatureLimit(int motor, double value) override;
    bool getGearboxRatio(int motor, double* value) override;
    bool setGearboxRatio(int motor, double value) override;
    bool getTemperatures(double* values) override;
}
;

template <>
class InterfaceAdapter<yarp::dev::IJointFault, dinrail::IJointFault>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IJointFault, dinrail::IJointFault>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool getLastJointFault(int axis, int& fault, std::string& message) override;
}
;

template <>
class InterfaceAdapter<dinrail::IImpedanceAllSetPointsControl,
                       dinrail::IImpedanceAllSetPointsControl>
    : public InterfaceAdapterBase<dinrail::IImpedanceAllSetPointsControl,
                                  dinrail::IImpedanceAllSetPointsControl>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    bool setSetPoint(
        int j, double pos, double vel, double torque, double stiffness, double damping) override;
    bool setSetPoints(const dinrail::VectorProxy<const int>::Ref jointIndeces,
                      const dinrail::VectorProxy<const double>::Ref pos,
                      const dinrail::VectorProxy<const double>::Ref vel,
                      const dinrail::VectorProxy<const double>::Ref torque,
                      const dinrail::VectorProxy<const double>::Ref stiffness,
                      const dinrail::VectorProxy<const double>::Ref damping) override;
    bool setSetPoints(const dinrail::VectorProxy<const double>::Ref pos,
                      const dinrail::VectorProxy<const double>::Ref vel,
                      const dinrail::VectorProxy<const double>::Ref torque,
                      const dinrail::VectorProxy<const double>::Ref stiffness,
                      const dinrail::VectorProxy<const double>::Ref damping) override;
    bool
    getSetPoint(int j, double& pos, double& vel, double& torque, double& stiffness, double& damping)
        override;
    bool getSetPoints(const dinrail::VectorProxy<const int>::Ref jointIndeces,
                      dinrail::VectorProxy<double>::Ref pos,
                      dinrail::VectorProxy<double>::Ref vel,
                      dinrail::VectorProxy<double>::Ref torque,
                      dinrail::VectorProxy<double>::Ref stiffness,
                      dinrail::VectorProxy<double>::Ref damping) override;
    bool getSetPoints(dinrail::VectorProxy<double>::Ref pos,
                      dinrail::VectorProxy<double>::Ref vel,
                      dinrail::VectorProxy<double>::Ref torque,
                      dinrail::VectorProxy<double>::Ref stiffness,
                      dinrail::VectorProxy<double>::Ref damping) override;
}
;

template <>
class InterfaceAdapter<dinrail::IAxisInfo, yarp::dev::IAxisInfo>
    : public dinrail::InterfaceAdapterBase<dinrail::IAxisInfo, yarp::dev::IAxisInfo>
{
public:
    explicit InterfaceAdapter(yarp::dev::IAxisInfo& source);

    bool getAxes(int* axes) override;
    bool getAxisName(int axis, std::string& name) override;
    bool getJointType(int axis, dinrail::JointType& type) override;

private:
    yarp::dev::IAxisInfo& m_source;
}
;

template <>
class InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>
    : public dinrail::InterfaceAdapterBase<dinrail::IEncoders, yarp::dev::IEncodersTimed>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    bool getAxes(int* axes) override;
    bool getEncoder(int joint, double* value) override;
    bool getEncoderTimed(int joint, double* value, double* timestamp) override;
    bool getEncoderSpeed(int joint, double* speed) override;
    bool getEncoderAcceleration(int joint, double* acceleration) override;
    bool getEncoders(dinrail::VectorProxy<double>::Ref values) override;
    bool getEncodersTimed(dinrail::VectorProxy<double>::Ref values,
                          dinrail::VectorProxy<double>::Ref timestamps) override;
    bool getEncoderSpeeds(dinrail::VectorProxy<double>::Ref speeds) override;
    bool getEncoderAccelerations(dinrail::VectorProxy<double>::Ref accelerations) override;
}
;

template <>
class InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>
    : public dinrail::InterfaceAdapterBase<dinrail::IEncoders, yarp::dev::IEncoders>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    bool getAxes(int* axes) override;
    bool getEncoder(int joint, double* value) override;
    bool getEncoderSpeed(int joint, double* speed) override;
    bool getEncoderAcceleration(int joint, double* acceleration) override;
    bool getEncoders(dinrail::VectorProxy<double>::Ref values) override;
    bool
    getEncodersTimed(dinrail::VectorProxy<double>::Ref, dinrail::VectorProxy<double>::Ref) override;
    bool getEncoderSpeeds(dinrail::VectorProxy<double>::Ref speeds) override;
    bool getEncoderAccelerations(dinrail::VectorProxy<double>::Ref accelerations) override;
    bool getEncoderTimed(int, double*, double*) override;
}
;

template <>
class InterfaceAdapter<dinrail::IJointFault, yarp::dev::IJointFault>
    : public dinrail::InterfaceAdapterBase<dinrail::IJointFault, yarp::dev::IJointFault>
{
public:
    explicit InterfaceAdapter(yarp::dev::IJointFault& source);
    bool getLastJointFault(int joint, int& fault, std::string& message) override;

private:
    yarp::dev::IJointFault& m_source;
}
;

template <>
class InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>
    : public dinrail::InterfaceAdapterBase<dinrail::IMotor, yarp::dev::IMotor>
{
public:
    explicit InterfaceAdapter(yarp::dev::IMotor& source);

    bool getNumberOfMotors(int* number) override;
    bool getTemperature(int motor, double* value) override;
    bool getTemperatureLimit(int motor, double* temperature) override;
    bool setTemperatureLimit(int motor, double temperature) override;
    bool getGearboxRatio(int motor, double* value) override;
    bool setGearboxRatio(int motor, double value) override;
    bool getTemperatures(dinrail::VectorProxy<double>::Ref values) override;

private:
    yarp::dev::IMotor& m_source;
}
;

template <>
class InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>
    : public dinrail::InterfaceAdapterBase<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>
{
public:
    explicit InterfaceAdapter(yarp::dev::IMotorEncoders& source);

    bool getNumberOfMotorEncoders(int* number) override;
    bool getMotorEncoderCountsPerRevolution(int motor, double* counts) override;
    bool getMotorEncoder(int motor, double* value) override;
    bool getMotorEncoderTimed(int motor, double* value, double* timestamp) override;
    bool getMotorEncoderSpeed(int motor, double* speed) override;
    bool getMotorEncoderAcceleration(int motor, double* acceleration) override;
    bool getMotorEncoders(dinrail::VectorProxy<double>::Ref values) override;
    bool getMotorEncodersTimed(dinrail::VectorProxy<double>::Ref values,
                               dinrail::VectorProxy<double>::Ref timestamps) override;
    bool getMotorEncoderSpeeds(dinrail::VectorProxy<double>::Ref speeds) override;
    bool getMotorEncoderAccelerations(dinrail::VectorProxy<double>::Ref accelerations) override;

private:
    template <class Read> bool readAll(dinrail::VectorProxy<double>::Ref output, Read&& read);

    yarp::dev::IMotorEncoders& m_source;
}
;

template <>
class InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IEncoders, dinrail::IEncoders>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    bool getAxes(int* count) override;
    bool getEncoder(int axis, double* value) override;
    bool getEncoderSpeed(int axis, double* value) override;
    bool getEncoderAcceleration(int axis, double* value) override;
    bool getEncoders(double* values) override;
    bool getEncoderSpeeds(double* values) override;
    bool getEncoderAccelerations(double* values) override;
    bool resetEncoder(int) override;
    bool resetEncoders() override;
    bool setEncoder(int, double) override;
    bool setEncoders(const double*) override;
}
;

} // namespace dinrail

#endif
