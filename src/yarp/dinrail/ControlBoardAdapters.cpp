// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/ControlBoardAdapters.h>

#include <dinrail/IAxisInfo.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IJointFault.h>
#include <dinrail/IMotor.h>
#include <dinrail/IMotorEncoders.h>

#include <yarp/dev/IAxisInfo.h>
#include <yarp/dev/IEncoders.h>
#include <yarp/dev/IEncodersTimed.h>
#include <yarp/dev/IJointFault.h>
#include <yarp/dev/IMotor.h>
#include <yarp/dev/IMotorEncoders.h>

namespace dinrail
{

bool InterfaceAdapter<yarp::dev::IAxisInfo, dinrail::IAxisInfo>::getAxes(int* axes)
{
    return source().getAxes(axes);
}

bool InterfaceAdapter<yarp::dev::IAxisInfo, dinrail::IAxisInfo>::getAxisName(int axis,
                                                                             std::string& name)
{
    return source().getAxisName(axis, name);
}

bool InterfaceAdapter<yarp::dev::IAxisInfo, dinrail::IAxisInfo>::getJointType(
    int axis, yarp::dev::JointTypeEnum& type)
{
    dinrail::JointType value;
    if (!source().getJointType(axis, value))
    {
        return false;
    }
    switch (value)
    {
    case dinrail::JointType::REVOLUTE:
        type = yarp::dev::VOCAB_JOINTTYPE_REVOLUTE;
        break;
    case dinrail::JointType::PRISMATIC:
        type = yarp::dev::VOCAB_JOINTTYPE_PRISMATIC;
        break;
    default:
        type = yarp::dev::VOCAB_JOINTTYPE_UNKNOWN;
        break;
    }
    return true;
}

bool InterfaceAdapter<yarp::dev::IJointFault, dinrail::IJointFault>::getLastJointFault(
    int axis, int& fault, std::string& message)
{
    return source().getLastJointFault(axis, fault, message);
}

bool prepareOutput(dinrail::VectorProxy<double>::Ref& output, int size)
{
    if (size < 0)
    {
        return false;
    }
    const auto expectedSize = static_cast<dinrail::VectorProxy<double>::index_type>(size);
    return output.size() == expectedSize || output.resizeVector(expectedSize);
}

    bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getAxes(int* count)
{
        return source().getAxes(count);
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getEncoder(int axis, double* value)
{
        return source().getEncoder(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getEncoderSpeed(int axis, double* value)
{
        return source().getEncoderSpeed(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getEncoderAcceleration(int axis, double* value)
{
        return source().getEncoderAcceleration(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getEncoderTimed(int axis, double* value, double* timestamp)
{
        return source().getEncoderTimed(axis, value, timestamp);
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getEncoders(double* values)
{
        int size = 0;
        if (!source().getAxes(&size) || size < 0 || (size > 0 && values == nullptr))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        return source().getEncoders(output);
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getEncoderSpeeds(double* values)
{
        int size = 0;
        if (!source().getAxes(&size) || size < 0 || (size > 0 && values == nullptr))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        return source().getEncoderSpeeds(output);
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getEncoderAccelerations(double* values)
{
        int size = 0;
        if (!source().getAxes(&size) || size < 0 || (size > 0 && values == nullptr))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        return source().getEncoderAccelerations(output);
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::getEncodersTimed(double* values, double* timestamps)
{
        int size = 0;
        if (!source().getAxes(&size) || size < 0
            || (size > 0 && (values == nullptr || timestamps == nullptr)))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        auto times = std::span(timestamps, static_cast<std::size_t>(size));
        return source().getEncodersTimed(output, times);
    }

// Calibration and reset operations were deliberately dropped from dinrail's measurement
    // interface.
    bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::resetEncoder(int)
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::resetEncoders()
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::setEncoder(int, double)
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IEncodersTimed, dinrail::IEncoders>::setEncoders(const double*)
{
        return false;
    }

    bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getNumberOfMotorEncoders(int* count)
{
        return source().getNumberOfMotorEncoders(count);
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncoder(int axis, double* value)
{
        return source().getMotorEncoder(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncoderSpeed(int axis, double* value)
{
        return source().getMotorEncoderSpeed(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncoderAcceleration(int axis, double* value)
{
        return source().getMotorEncoderAcceleration(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncoderTimed(int axis, double* value, double* timestamp)
{
        return source().getMotorEncoderTimed(axis, value, timestamp);
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncoders(double* values)
{
        int size = 0;
        if (!source().getNumberOfMotorEncoders(&size) || size < 0
            || (size > 0 && values == nullptr))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        return source().getMotorEncoders(output);
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncoderSpeeds(double* values)
{
        int size = 0;
        if (!source().getNumberOfMotorEncoders(&size) || size < 0
            || (size > 0 && values == nullptr))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        return source().getMotorEncoderSpeeds(output);
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncoderAccelerations(double* values)
{
        int size = 0;
        if (!source().getNumberOfMotorEncoders(&size) || size < 0
            || (size > 0 && values == nullptr))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        return source().getMotorEncoderAccelerations(output);
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncodersTimed(double* values, double* timestamps)
{
        int size = 0;
        if (!source().getNumberOfMotorEncoders(&size) || size < 0
            || (size > 0 && (values == nullptr || timestamps == nullptr)))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        auto times = std::span(timestamps, static_cast<std::size_t>(size));
        return source().getMotorEncodersTimed(output, times);
    }

// Calibration and reset operations were deliberately dropped from dinrail's measurement
    // interface.
    bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::resetMotorEncoder(int)
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::resetMotorEncoders()
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::setMotorEncoder(int, double)
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::setMotorEncoders(const double*)
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::setMotorEncoderCountsPerRevolution(int, double)
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IMotorEncoders, dinrail::IMotorEncoders>::getMotorEncoderCountsPerRevolution(int axis, double* counts)
{
        return source().getMotorEncoderCountsPerRevolution(axis, counts);
    }

    bool InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>::getNumberOfMotors(int* count)
{
        return source().getNumberOfMotors(count);
    }

bool InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>::getTemperature(int motor, double* value)
{
        return source().getTemperature(motor, value);
    }

bool InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>::getTemperatureLimit(int motor, double* value)
{
        return source().getTemperatureLimit(motor, value);
    }

bool InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>::setTemperatureLimit(int motor, double value)
{
        return source().setTemperatureLimit(motor, value);
    }

bool InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>::getGearboxRatio(int motor, double* value)
{
        return source().getGearboxRatio(motor, value);
    }

bool InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>::setGearboxRatio(int motor, double value)
{
        return source().setGearboxRatio(motor, value);
    }

bool InterfaceAdapter<yarp::dev::IMotor, dinrail::IMotor>::getTemperatures(double* values)
{
        int size = 0;
        if (!source().getNumberOfMotors(&size) || size < 0 || (size > 0 && values == nullptr))
        {
            return false;
        }
        auto output = std::span(values, static_cast<std::size_t>(size));
        return source().getTemperatures(output);
    }

    bool InterfaceAdapter<dinrail::IImpedanceAllSetPointsControl,
                       dinrail::IImpedanceAllSetPointsControl>::setSetPoint(
        int j, double pos, double vel, double torque, double stiffness, double damping)
{
        return source().setSetPoint(j, pos, vel, torque, stiffness, damping);
    }

bool InterfaceAdapter<dinrail::IImpedanceAllSetPointsControl,
                       dinrail::IImpedanceAllSetPointsControl>::setSetPoints(const dinrail::VectorProxy<const int>::Ref jointIndeces,
                      const dinrail::VectorProxy<const double>::Ref pos,
                      const dinrail::VectorProxy<const double>::Ref vel,
                      const dinrail::VectorProxy<const double>::Ref torque,
                      const dinrail::VectorProxy<const double>::Ref stiffness,
                      const dinrail::VectorProxy<const double>::Ref damping)
{
        return source().setSetPoints(jointIndeces, pos, vel, torque, stiffness, damping);
    }

bool InterfaceAdapter<dinrail::IImpedanceAllSetPointsControl,
                       dinrail::IImpedanceAllSetPointsControl>::setSetPoints(const dinrail::VectorProxy<const double>::Ref pos,
                      const dinrail::VectorProxy<const double>::Ref vel,
                      const dinrail::VectorProxy<const double>::Ref torque,
                      const dinrail::VectorProxy<const double>::Ref stiffness,
                      const dinrail::VectorProxy<const double>::Ref damping)
{
        return source().setSetPoints(pos, vel, torque, stiffness, damping);
    }

bool
    InterfaceAdapter<dinrail::IImpedanceAllSetPointsControl,
                       dinrail::IImpedanceAllSetPointsControl>::getSetPoint(int j, double& pos, double& vel, double& torque, double& stiffness, double& damping)
{
        return source().getSetPoint(j, pos, vel, torque, stiffness, damping);
    }

bool InterfaceAdapter<dinrail::IImpedanceAllSetPointsControl,
                       dinrail::IImpedanceAllSetPointsControl>::getSetPoints(const dinrail::VectorProxy<const int>::Ref jointIndeces,
                      dinrail::VectorProxy<double>::Ref pos,
                      dinrail::VectorProxy<double>::Ref vel,
                      dinrail::VectorProxy<double>::Ref torque,
                      dinrail::VectorProxy<double>::Ref stiffness,
                      dinrail::VectorProxy<double>::Ref damping)
{
        return source().getSetPoints(jointIndeces, pos, vel, torque, stiffness, damping);
    }

bool InterfaceAdapter<dinrail::IImpedanceAllSetPointsControl,
                       dinrail::IImpedanceAllSetPointsControl>::getSetPoints(dinrail::VectorProxy<double>::Ref pos,
                      dinrail::VectorProxy<double>::Ref vel,
                      dinrail::VectorProxy<double>::Ref torque,
                      dinrail::VectorProxy<double>::Ref stiffness,
                      dinrail::VectorProxy<double>::Ref damping)
{
        return source().getSetPoints(pos, vel, torque, stiffness, damping);
    }

InterfaceAdapter<dinrail::IAxisInfo, yarp::dev::IAxisInfo>::InterfaceAdapter(yarp::dev::IAxisInfo& source)
    : InterfaceAdapterBase(source)
    , m_source(source)
{
    }

bool InterfaceAdapter<dinrail::IAxisInfo, yarp::dev::IAxisInfo>::getAxes(int* axes)
{
        return m_source.getAxes(axes);
    }

bool InterfaceAdapter<dinrail::IAxisInfo, yarp::dev::IAxisInfo>::getAxisName(int axis, std::string& name)
{
        return m_source.getAxisName(axis, name);
    }

bool InterfaceAdapter<dinrail::IAxisInfo, yarp::dev::IAxisInfo>::getJointType(int axis, dinrail::JointType& type)
{
        yarp::dev::JointTypeEnum yarpType = yarp::dev::VOCAB_JOINTTYPE_UNKNOWN;
        if (!m_source.getJointType(axis, yarpType))
        {
            return false;
        }

        switch (yarpType)
        {
        case yarp::dev::VOCAB_JOINTTYPE_REVOLUTE:
            type = dinrail::JointType::REVOLUTE;
            break;
        case yarp::dev::VOCAB_JOINTTYPE_PRISMATIC:
            type = dinrail::JointType::PRISMATIC;
            break;
        default:
            type = dinrail::JointType::UNKNOWN;
            break;
        }
        return true;
    }

    bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getAxes(int* axes)
{
        return source().getAxes(axes);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getEncoder(int joint, double* value)
{
        return source().getEncoder(joint, value);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getEncoderTimed(int joint, double* value, double* timestamp)
{
        return source().getEncoderTimed(joint, value, timestamp);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getEncoderSpeed(int joint, double* speed)
{
        return source().getEncoderSpeed(joint, speed);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getEncoderAcceleration(int joint, double* acceleration)
{
        return source().getEncoderAcceleration(joint, acceleration);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getEncoders(dinrail::VectorProxy<double>::Ref values)
{
        int axes = 0;
        return source().getAxes(&axes) && prepareOutput(values, axes)
               && source().getEncoders(values.data());
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getEncodersTimed(dinrail::VectorProxy<double>::Ref values,
                          dinrail::VectorProxy<double>::Ref timestamps)
{
        int axes = 0;
        return source().getAxes(&axes) && prepareOutput(values, axes)
               && prepareOutput(timestamps, axes)
               && source().getEncodersTimed(values.data(), timestamps.data());
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getEncoderSpeeds(dinrail::VectorProxy<double>::Ref speeds)
{
        int axes = 0;
        return source().getAxes(&axes) && prepareOutput(speeds, axes)
               && source().getEncoderSpeeds(speeds.data());
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncodersTimed>::getEncoderAccelerations(dinrail::VectorProxy<double>::Ref accelerations)
{
        int axes = 0;
        return source().getAxes(&axes) && prepareOutput(accelerations, axes)
               && source().getEncoderAccelerations(accelerations.data());
    }

    bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getAxes(int* axes)
{
        return source().getAxes(axes);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getEncoder(int joint, double* value)
{
        return source().getEncoder(joint, value);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getEncoderSpeed(int joint, double* speed)
{
        return source().getEncoderSpeed(joint, speed);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getEncoderAcceleration(int joint, double* acceleration)
{
        return source().getEncoderAcceleration(joint, acceleration);
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getEncoders(dinrail::VectorProxy<double>::Ref values)
{
        int axes = 0;
        return source().getAxes(&axes) && prepareOutput(values, axes)
               && source().getEncoders(values.data());
    }

bool
    InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getEncodersTimed(dinrail::VectorProxy<double>::Ref, dinrail::VectorProxy<double>::Ref)
{
        return false;
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getEncoderSpeeds(dinrail::VectorProxy<double>::Ref speeds)
{
        int axes = 0;
        return source().getAxes(&axes) && prepareOutput(speeds, axes)
               && source().getEncoderSpeeds(speeds.data());
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getEncoderAccelerations(dinrail::VectorProxy<double>::Ref accelerations)
{
        int axes = 0;
        return source().getAxes(&axes) && prepareOutput(accelerations, axes)
               && source().getEncoderAccelerations(accelerations.data());
    }

bool InterfaceAdapter<dinrail::IEncoders, yarp::dev::IEncoders>::getEncoderTimed(int, double*, double*)
{
        return false;
    }

InterfaceAdapter<dinrail::IJointFault, yarp::dev::IJointFault>::InterfaceAdapter(yarp::dev::IJointFault& source)
    : InterfaceAdapterBase(source)
    , m_source(source)
{
    }

bool InterfaceAdapter<dinrail::IJointFault, yarp::dev::IJointFault>::getLastJointFault(int joint, int& fault, std::string& message)
{
        return m_source.getLastJointFault(joint, fault, message);
    }

InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>::InterfaceAdapter(yarp::dev::IMotor& source)
    : InterfaceAdapterBase(source)
    , m_source(source)
{
    }

bool InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>::getNumberOfMotors(int* number)
{
        return m_source.getNumberOfMotors(number);
    }

bool InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>::getTemperature(int motor, double* value)
{
        return m_source.getTemperature(motor, value);
    }

bool InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>::getTemperatureLimit(int motor, double* temperature)
{
        return m_source.getTemperatureLimit(motor, temperature);
    }

bool InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>::setTemperatureLimit(int motor, double temperature)
{
        return m_source.setTemperatureLimit(motor, temperature);
    }

bool InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>::getGearboxRatio(int motor, double* value)
{
        return m_source.getGearboxRatio(motor, value);
    }

bool InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>::setGearboxRatio(int motor, double value)
{
        return m_source.setGearboxRatio(motor, value);
    }

bool InterfaceAdapter<dinrail::IMotor, yarp::dev::IMotor>::getTemperatures(dinrail::VectorProxy<double>::Ref values)
{
        int motors = 0;
        return m_source.getNumberOfMotors(&motors) && prepareOutput(values, motors)
               && m_source.getTemperatures(values.data());
    }

InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::InterfaceAdapter(yarp::dev::IMotorEncoders& source)
    : InterfaceAdapterBase(source)
    , m_source(source)
{
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getNumberOfMotorEncoders(int* number)
{
        return m_source.getNumberOfMotorEncoders(number);
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncoderCountsPerRevolution(int motor, double* counts)
{
        return m_source.getMotorEncoderCountsPerRevolution(motor, counts);
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncoder(int motor, double* value)
{
        return m_source.getMotorEncoder(motor, value);
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncoderTimed(int motor, double* value, double* timestamp)
{
        return m_source.getMotorEncoderTimed(motor, value, timestamp);
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncoderSpeed(int motor, double* speed)
{
        return m_source.getMotorEncoderSpeed(motor, speed);
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncoderAcceleration(int motor, double* acceleration)
{
        return m_source.getMotorEncoderAcceleration(motor, acceleration);
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncoders(dinrail::VectorProxy<double>::Ref values)
{
        return readAll(values, [&](double* data) { return m_source.getMotorEncoders(data); });
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncodersTimed(dinrail::VectorProxy<double>::Ref values,
                               dinrail::VectorProxy<double>::Ref timestamps)
{
        int motors = 0;
        return m_source.getNumberOfMotorEncoders(&motors) && prepareOutput(values, motors)
               && prepareOutput(timestamps, motors)
               && m_source.getMotorEncodersTimed(values.data(), timestamps.data());
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncoderSpeeds(dinrail::VectorProxy<double>::Ref speeds)
{
        return readAll(speeds, [&](double* data) { return m_source.getMotorEncoderSpeeds(data); });
    }

bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::getMotorEncoderAccelerations(dinrail::VectorProxy<double>::Ref accelerations)
{
        return readAll(accelerations,
                       [&](double* data) { return m_source.getMotorEncoderAccelerations(data); });
    }

template <class Read> bool InterfaceAdapter<dinrail::IMotorEncoders, yarp::dev::IMotorEncoders>::readAll(dinrail::VectorProxy<double>::Ref output, Read&& read)
{
        int motors = 0;
        return m_source.getNumberOfMotorEncoders(&motors) && prepareOutput(output, motors)
               && read(output.data());
    }

    bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::getAxes(int* count)
{
        return source().getAxes(count);
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::getEncoder(int axis, double* value)
{
        return source().getEncoder(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::getEncoderSpeed(int axis, double* value)
{
        return source().getEncoderSpeed(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::getEncoderAcceleration(int axis, double* value)
{
        return source().getEncoderAcceleration(axis, value);
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::getEncoders(double* values)
{
        int size = 0;
        if (!source().getAxes(&size) || size < 0 || (size > 0 && values == nullptr))
        {
            return false;
        }
        dinrail::VectorProxy<double> output(std::span(values, static_cast<std::size_t>(size)));
        return source().getEncoders(output);
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::getEncoderSpeeds(double* values)
{
        int size = 0;
        if (!source().getAxes(&size) || size < 0 || (size > 0 && values == nullptr))
        {
            return false;
        }
        dinrail::VectorProxy<double> output(std::span(values, static_cast<std::size_t>(size)));
        return source().getEncoderSpeeds(output);
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::getEncoderAccelerations(double* values)
{
        int size = 0;
        if (!source().getAxes(&size) || size < 0 || (size > 0 && values == nullptr))
        {
            return false;
        }
        dinrail::VectorProxy<double> output(std::span(values, static_cast<std::size_t>(size)));
        return source().getEncoderAccelerations(output);
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::resetEncoder(int)
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::resetEncoders()
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::setEncoder(int, double)
{
        return false;
    }

bool InterfaceAdapter<yarp::dev::IEncoders, dinrail::IEncoders>::setEncoders(const double*)
{
        return false;
    }

} // namespace dinrail
