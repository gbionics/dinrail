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
    std::transform(normalized.begin(),
                   normalized.end(),
                   normalized.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

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
    if (!config.check<int>("number_of_joints") && !general.isNull() && general.find("Joints").isInt())
    {
        // Deprecated compatibility path with YARP fakeMotionControl.
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

bool FakeMotionControl::setSetPoint(int j,
                                    double pos,
                                    double vel,
                                    double torque,
                                    double stiffness,
                                    double damping)
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

bool FakeMotionControl::getSetPoint(int j,
                                    double& pos,
                                    double& vel,
                                    double& torque,
                                    double& stiffness,
                                    double& damping)
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
