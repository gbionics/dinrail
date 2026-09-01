// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "FakeMotionControl.h"

#include <algorithm>
#include <cctype>

namespace dinrail
{

namespace
{
bool isValidJointIndex(int axis, int njoints)
{
    return axis >= 0 && axis < njoints;
}

bool parseJointType(const std::string& typeString, JointType& type)
{
    std::string normalized = typeString;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (normalized == "revolute")
    {
        type = JointType::REVOLUTE;
        return true;
    }

    if (normalized == "prismatic")
    {
        type = JointType::PRISMATIC;
        return true;
    }

    return false;
}

bool prepareEncoderOutput(VectorProxy<double>::Ref output, int njoints)
{
    const auto expectedSize = static_cast<std::ptrdiff_t>(njoints);
    return output.size() == expectedSize || output.resizeVector(expectedSize);
}
} // namespace

bool FakeMotionControl::open(const Parameters& config)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_njoints = 1;

    if (config.check<int>("number_of_joints"))
    {
        m_njoints = config.find("number_of_joints").as<int>();
    }

    const Parameters& general = config.findGroup("GENERAL");
    if (!config.check<int>("number_of_joints") && !general.isNull()
        && general.find("Joints").isInt())
    {
        // Deprecated compatibility path with YARP dr_controlboard_fake.
        m_njoints = general.find("Joints").as<int>();
    }

    if (m_njoints <= 0)
    {
        return false;
    }

    m_axisNames.clear();
    m_axisNames.reserve(static_cast<std::size_t>(m_njoints));
    m_jointTypes.clear();
    m_jointTypes.assign(static_cast<std::size_t>(m_njoints), JointType::REVOLUTE);
    m_encoderPositions.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_encoderTimestamps.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_encoderSpeeds.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_encoderAccelerations.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_motorTemperatures.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_motorTemperatureLimits.assign(static_cast<std::size_t>(m_njoints), 100.0);
    m_motorGearboxRatios.assign(static_cast<std::size_t>(m_njoints), 1.0);
    m_motorEncoderCountsPerRevolution.assign(static_cast<std::size_t>(m_njoints), 1.0);
    m_jointFaults.assign(static_cast<std::size_t>(m_njoints), 0);
    m_jointFaultMessages.assign(static_cast<std::size_t>(m_njoints), std::string{});
    m_posSetpoints.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_velSetpoints.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_torqueSetpoints.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_stiffnessSetpoints.assign(static_cast<std::size_t>(m_njoints), 0.0);
    m_dampingSetpoints.assign(static_cast<std::size_t>(m_njoints), 0.0);

    for (int j = 0; j < m_njoints; ++j)
    {
        m_axisNames.push_back("joint_" + std::to_string(j));
    }

    if (config.check<std::vector<std::string>>("joint_names"))
    {
        const auto& names = config.find("joint_names").as<std::vector<std::string>>();
        if (names.size() != static_cast<std::size_t>(m_njoints))
        {
            return false;
        }

        m_axisNames = names;
    }

    if (config.check<std::vector<std::string>>("joint_type"))
    {
        const auto& typeStrings = config.find("joint_type").as<std::vector<std::string>>();
        if (typeStrings.size() != static_cast<std::size_t>(m_njoints))
        {
            return false;
        }

        for (std::size_t i = 0; i < typeStrings.size(); ++i)
        {
            JointType parsedType = JointType::UNKNOWN;
            if (!parseJointType(typeStrings[i], parsedType))
            {
                return false;
            }

            m_jointTypes[i] = parsedType;
        }
    }

    m_opened = true;
    return true;
}

bool FakeMotionControl::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_opened = false;
    return true;
}

bool FakeMotionControl::getAxes(int* ax)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || ax == nullptr)
    {
        return false;
    }

    *ax = m_njoints;
    return true;
}

bool FakeMotionControl::getAxisName(int axis, std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || axis < 0 || axis >= m_njoints)
    {
        return false;
    }

    name = m_axisNames[static_cast<std::size_t>(axis)];
    return true;
}

bool FakeMotionControl::getJointType(int axis, JointType& type)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(axis, m_njoints))
    {
        return false;
    }

    type = m_jointTypes[static_cast<std::size_t>(axis)];
    return true;
}

bool FakeMotionControl::getEncoder(int j, double* value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints) || value == nullptr)
    {
        return false;
    }

    *value = m_encoderPositions[static_cast<std::size_t>(j)];
    return true;
}

bool FakeMotionControl::getEncoders(VectorProxy<double>::Ref values)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(values, m_njoints))
    {
        return false;
    }

    return values.clone(std::span<double>(m_encoderPositions));
}

bool FakeMotionControl::getEncoderTimed(int j, double* value, double* timestamp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints) || value == nullptr || timestamp == nullptr)
    {
        return false;
    }

    const auto index = static_cast<std::size_t>(j);
    *value = m_encoderPositions[index];
    *timestamp = m_encoderTimestamps[index];
    return true;
}

bool FakeMotionControl::getEncodersTimed(VectorProxy<double>::Ref values,
                                         VectorProxy<double>::Ref timestamps)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(values, m_njoints)
        || !prepareEncoderOutput(timestamps, m_njoints))
    {
        return false;
    }

    return values.clone(std::span<double>(m_encoderPositions))
           && timestamps.clone(std::span<double>(m_encoderTimestamps));
}

bool FakeMotionControl::getEncoderSpeed(int j, double* speed)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints) || speed == nullptr)
    {
        return false;
    }

    *speed = m_encoderSpeeds[static_cast<std::size_t>(j)];
    return true;
}

bool FakeMotionControl::getEncoderSpeeds(VectorProxy<double>::Ref speeds)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(speeds, m_njoints))
    {
        return false;
    }

    return speeds.clone(std::span<double>(m_encoderSpeeds));
}

bool FakeMotionControl::getEncoderAcceleration(int j, double* acceleration)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints) || acceleration == nullptr)
    {
        return false;
    }

    *acceleration = m_encoderAccelerations[static_cast<std::size_t>(j)];
    return true;
}

bool FakeMotionControl::getEncoderAccelerations(VectorProxy<double>::Ref accelerations)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(accelerations, m_njoints))
    {
        return false;
    }

    return accelerations.clone(std::span<double>(m_encoderAccelerations));
}

bool FakeMotionControl::getNumberOfMotors(int* num)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || num == nullptr)
    {
        return false;
    }

    *num = m_njoints;
    return true;
}

bool FakeMotionControl::getTemperature(int m, double* value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints) || value == nullptr)
    {
        return false;
    }

    *value = m_motorTemperatures[static_cast<std::size_t>(m)];
    return true;
}

bool FakeMotionControl::getTemperatures(VectorProxy<double>::Ref values)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(values, m_njoints))
    {
        return false;
    }

    return values.clone(std::span<double>(m_motorTemperatures));
}

bool FakeMotionControl::getTemperatureLimit(int m, double* temperature)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints) || temperature == nullptr)
    {
        return false;
    }

    *temperature = m_motorTemperatureLimits[static_cast<std::size_t>(m)];
    return true;
}

bool FakeMotionControl::setTemperatureLimit(int m, double temperature)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints))
    {
        return false;
    }

    m_motorTemperatureLimits[static_cast<std::size_t>(m)] = temperature;
    return true;
}

bool FakeMotionControl::getGearboxRatio(int m, double* value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints) || value == nullptr)
    {
        return false;
    }

    *value = m_motorGearboxRatios[static_cast<std::size_t>(m)];
    return true;
}

bool FakeMotionControl::setGearboxRatio(int m, double value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints))
    {
        return false;
    }

    m_motorGearboxRatios[static_cast<std::size_t>(m)] = value;
    return true;
}

bool FakeMotionControl::setTemperature(int m, double value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints))
    {
        return false;
    }

    m_motorTemperatures[static_cast<std::size_t>(m)] = value;
    return true;
}

bool FakeMotionControl::setTemperatures(const VectorProxy<const double>::Ref values)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || values.size() != static_cast<std::ptrdiff_t>(m_njoints))
    {
        return false;
    }

    std::copy(values.begin(), values.end(), m_motorTemperatures.begin());
    return true;
}

bool FakeMotionControl::getNumberOfMotorEncoders(int* num)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || num == nullptr)
    {
        return false;
    }

    *num = m_njoints;
    return true;
}

bool FakeMotionControl::getMotorEncoderCountsPerRevolution(int m, double* countsPerRevolution)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints) || countsPerRevolution == nullptr)
    {
        return false;
    }

    *countsPerRevolution = m_motorEncoderCountsPerRevolution[static_cast<std::size_t>(m)];
    return true;
}

bool FakeMotionControl::getMotorEncoder(int m, double* value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints) || value == nullptr)
    {
        return false;
    }

    *value = m_encoderPositions[static_cast<std::size_t>(m)];
    return true;
}

bool FakeMotionControl::getMotorEncoders(VectorProxy<double>::Ref values)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(values, m_njoints))
    {
        return false;
    }

    return values.clone(std::span<double>(m_encoderPositions));
}

bool FakeMotionControl::getMotorEncoderTimed(int m, double* value, double* timestamp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints) || value == nullptr || timestamp == nullptr)
    {
        return false;
    }

    const auto index = static_cast<std::size_t>(m);
    *value = m_encoderPositions[index];
    *timestamp = m_encoderTimestamps[index];
    return true;
}

bool FakeMotionControl::getMotorEncodersTimed(VectorProxy<double>::Ref values,
                                              VectorProxy<double>::Ref timestamps)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(values, m_njoints)
        || !prepareEncoderOutput(timestamps, m_njoints))
    {
        return false;
    }

    return values.clone(std::span<double>(m_encoderPositions))
           && timestamps.clone(std::span<double>(m_encoderTimestamps));
}

bool FakeMotionControl::getMotorEncoderSpeed(int m, double* speed)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints) || speed == nullptr)
    {
        return false;
    }

    *speed = m_encoderSpeeds[static_cast<std::size_t>(m)];
    return true;
}

bool FakeMotionControl::getMotorEncoderSpeeds(VectorProxy<double>::Ref speeds)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(speeds, m_njoints))
    {
        return false;
    }

    return speeds.clone(std::span<double>(m_encoderSpeeds));
}

bool FakeMotionControl::getMotorEncoderAcceleration(int m, double* acceleration)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(m, m_njoints) || acceleration == nullptr)
    {
        return false;
    }

    *acceleration = m_encoderAccelerations[static_cast<std::size_t>(m)];
    return true;
}

bool FakeMotionControl::getMotorEncoderAccelerations(VectorProxy<double>::Ref accelerations)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !prepareEncoderOutput(accelerations, m_njoints))
    {
        return false;
    }

    return accelerations.clone(std::span<double>(m_encoderAccelerations));
}

bool FakeMotionControl::setMotorEncoder(int m, double value)
{
    return setEncoder(m, value);
}

bool FakeMotionControl::setMotorEncoders(const VectorProxy<const double>::Ref values)
{
    return setEncoders(values);
}

bool FakeMotionControl::setMotorEncoderTimed(int m, double value, double timestamp)
{
    return setEncoderTimed(m, value, timestamp);
}

bool FakeMotionControl::setMotorEncodersTimed(const VectorProxy<const double>::Ref values,
                                              const VectorProxy<const double>::Ref timestamps)
{
    return setEncodersTimed(values, timestamps);
}

bool FakeMotionControl::setMotorEncoderSpeed(int m, double speed)
{
    return setEncoderSpeed(m, speed);
}

bool FakeMotionControl::setMotorEncoderSpeeds(const VectorProxy<const double>::Ref speeds)
{
    return setEncoderSpeeds(speeds);
}

bool FakeMotionControl::setMotorEncoderAcceleration(int m, double acceleration)
{
    return setEncoderAcceleration(m, acceleration);
}

bool FakeMotionControl::setMotorEncoderAccelerations(
    const VectorProxy<const double>::Ref accelerations)
{
    return setEncoderAccelerations(accelerations);
}

bool FakeMotionControl::getLastJointFault(int j, int& fault, std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints))
    {
        return false;
    }

    const auto index = static_cast<std::size_t>(j);
    fault = m_jointFaults[index];
    message = m_jointFaultMessages[index];
    return true;
}

bool FakeMotionControl::setLastJointFault(int j, int fault, const std::string& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints))
    {
        return false;
    }

    const auto index = static_cast<std::size_t>(j);
    m_jointFaults[index] = fault;
    m_jointFaultMessages[index] = message;
    return true;
}

bool FakeMotionControl::setEncoder(int j, double value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints))
    {
        return false;
    }

    m_encoderPositions[static_cast<std::size_t>(j)] = value;
    return true;
}

bool FakeMotionControl::setEncoders(const VectorProxy<const double>::Ref values)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || values.size() != static_cast<std::ptrdiff_t>(m_njoints))
    {
        return false;
    }

    std::copy(values.begin(), values.end(), m_encoderPositions.begin());
    return true;
}

bool FakeMotionControl::setEncoderTimed(int j, double value, double timestamp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints))
    {
        return false;
    }

    const auto index = static_cast<std::size_t>(j);
    m_encoderPositions[index] = value;
    m_encoderTimestamps[index] = timestamp;
    return true;
}

bool FakeMotionControl::setEncodersTimed(const VectorProxy<const double>::Ref values,
                                         const VectorProxy<const double>::Ref timestamps)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto expectedSize = static_cast<std::ptrdiff_t>(m_njoints);
    if (!m_opened || values.size() != expectedSize || timestamps.size() != expectedSize)
    {
        return false;
    }

    std::copy(values.begin(), values.end(), m_encoderPositions.begin());
    std::copy(timestamps.begin(), timestamps.end(), m_encoderTimestamps.begin());
    return true;
}

bool FakeMotionControl::setEncoderSpeed(int j, double speed)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints))
    {
        return false;
    }

    m_encoderSpeeds[static_cast<std::size_t>(j)] = speed;
    return true;
}

bool FakeMotionControl::setEncoderSpeeds(const VectorProxy<const double>::Ref speeds)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || speeds.size() != static_cast<std::ptrdiff_t>(m_njoints))
    {
        return false;
    }

    std::copy(speeds.begin(), speeds.end(), m_encoderSpeeds.begin());
    return true;
}

bool FakeMotionControl::setEncoderAcceleration(int j, double acceleration)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints))
    {
        return false;
    }

    m_encoderAccelerations[static_cast<std::size_t>(j)] = acceleration;
    return true;
}

bool FakeMotionControl::setEncoderAccelerations(const VectorProxy<const double>::Ref accelerations)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || accelerations.size() != static_cast<std::ptrdiff_t>(m_njoints))
    {
        return false;
    }

    std::copy(accelerations.begin(), accelerations.end(), m_encoderAccelerations.begin());
    return true;
}

Stamp FakeMotionControl::getLastInputStamp()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_lastInputStamp;
}

void FakeMotionControl::setLastInputStamp(const Stamp& stamp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_lastInputStamp = stamp;
}

bool FakeMotionControl::setSetPoint(
    int j, double pos, double vel, double torque, double stiffness, double damping)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints))
    {
        return false;
    }

    const auto index = static_cast<std::size_t>(j);
    m_posSetpoints[index] = pos;
    m_velSetpoints[index] = vel;
    m_torqueSetpoints[index] = torque;
    m_stiffnessSetpoints[index] = stiffness;
    m_dampingSetpoints[index] = damping;
    return true;
}

bool FakeMotionControl::setSetPoints(const VectorProxy<const int>::Ref jointIndeces,
                                     const VectorProxy<const double>::Ref pos,
                                     const VectorProxy<const double>::Ref vel,
                                     const VectorProxy<const double>::Ref torque,
                                     const VectorProxy<const double>::Ref stiffness,
                                     const VectorProxy<const double>::Ref damping)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return false;
    }

    const auto inputSize = jointIndeces.size();
    if (inputSize != pos.size() || inputSize != vel.size() || inputSize != torque.size()
        || inputSize != stiffness.size() || inputSize != damping.size())
    {
        return false;
    }

    for (std::ptrdiff_t i = 0; i < inputSize; ++i)
    {
        if (!isValidJointIndex(jointIndeces[i], m_njoints))
        {
            return false;
        }
    }

    for (std::ptrdiff_t i = 0; i < inputSize; ++i)
    {
        const auto index = static_cast<std::size_t>(jointIndeces[i]);
        m_posSetpoints[index] = pos[i];
        m_velSetpoints[index] = vel[i];
        m_torqueSetpoints[index] = torque[i];
        m_stiffnessSetpoints[index] = stiffness[i];
        m_dampingSetpoints[index] = damping[i];
    }

    return true;
}

bool FakeMotionControl::setSetPoints(const VectorProxy<const double>::Ref pos,
                                     const VectorProxy<const double>::Ref vel,
                                     const VectorProxy<const double>::Ref torque,
                                     const VectorProxy<const double>::Ref stiffness,
                                     const VectorProxy<const double>::Ref damping)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return false;
    }

    const auto expectedSize = static_cast<std::ptrdiff_t>(m_njoints);
    if (expectedSize != pos.size() || expectedSize != vel.size() || expectedSize != torque.size()
        || expectedSize != stiffness.size() || expectedSize != damping.size())
    {
        return false;
    }

    for (std::ptrdiff_t i = 0; i < expectedSize; ++i)
    {
        const auto index = static_cast<std::size_t>(i);
        m_posSetpoints[index] = pos[i];
        m_velSetpoints[index] = vel[i];
        m_torqueSetpoints[index] = torque[i];
        m_stiffnessSetpoints[index] = stiffness[i];
        m_dampingSetpoints[index] = damping[i];
    }

    return true;
}

bool FakeMotionControl::getSetPoint(
    int j, double& pos, double& vel, double& torque, double& stiffness, double& damping)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened || !isValidJointIndex(j, m_njoints))
    {
        return false;
    }

    const auto index = static_cast<std::size_t>(j);
    pos = m_posSetpoints[index];
    vel = m_velSetpoints[index];
    torque = m_torqueSetpoints[index];
    stiffness = m_stiffnessSetpoints[index];
    damping = m_dampingSetpoints[index];
    return true;
}

bool FakeMotionControl::getSetPoints(const VectorProxy<const int>::Ref jointIndeces,
                                     VectorProxy<double>::Ref pos,
                                     VectorProxy<double>::Ref vel,
                                     VectorProxy<double>::Ref torque,
                                     VectorProxy<double>::Ref stiffness,
                                     VectorProxy<double>::Ref damping)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return false;
    }

    const auto outputSize = jointIndeces.size();
    if (outputSize != pos.size() || outputSize != vel.size() || outputSize != torque.size()
        || outputSize != stiffness.size() || outputSize != damping.size())
    {
        return false;
    }

    for (std::ptrdiff_t i = 0; i < outputSize; ++i)
    {
        if (!isValidJointIndex(jointIndeces[i], m_njoints))
        {
            return false;
        }
    }

    for (std::ptrdiff_t i = 0; i < outputSize; ++i)
    {
        const auto jointIndex = static_cast<std::size_t>(jointIndeces[i]);
        pos[i] = m_posSetpoints[jointIndex];
        vel[i] = m_velSetpoints[jointIndex];
        torque[i] = m_torqueSetpoints[jointIndex];
        stiffness[i] = m_stiffnessSetpoints[jointIndex];
        damping[i] = m_dampingSetpoints[jointIndex];
    }

    return true;
}

bool FakeMotionControl::getSetPoints(VectorProxy<double>::Ref pos,
                                     VectorProxy<double>::Ref vel,
                                     VectorProxy<double>::Ref torque,
                                     VectorProxy<double>::Ref stiffness,
                                     VectorProxy<double>::Ref damping)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_opened)
    {
        return false;
    }

    const auto expectedSize = static_cast<std::ptrdiff_t>(m_njoints);
    if (expectedSize != pos.size() || expectedSize != vel.size() || expectedSize != torque.size()
        || expectedSize != stiffness.size() || expectedSize != damping.size())
    {
        return false;
    }

    for (std::ptrdiff_t i = 0; i < expectedSize; ++i)
    {
        const auto index = static_cast<std::size_t>(i);
        pos[i] = m_posSetpoints[index];
        vel[i] = m_velSetpoints[index];
        torque[i] = m_torqueSetpoints[index];
        stiffness[i] = m_stiffnessSetpoints[index];
        damping[i] = m_dampingSetpoints[index];
    }

    return true;
}

} // namespace dinrail
