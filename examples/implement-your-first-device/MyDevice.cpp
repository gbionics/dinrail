// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "MyDevice.h"

bool MyDevice::open(const dinrail::Parameters& config)
{
    // Read the number of joints (default: 1).
    m_njoints = 1;
    if (config.check<int>("number_of_joints"))
    {
        m_njoints = config.find("number_of_joints").as<int>();
    }

    if (m_njoints <= 0)
    {
        return false;
    }

    m_gain = 1.0;
    if (config.check<double>("gain"))
    {
        const double gain = config.find("gain").as<double>();
        if (gain <= 0.0)
        {
            return false;
        }
        m_gain = gain;
    }

    // Populate default axis names.
    m_axisNames.clear();
    for (int j = 0; j < m_njoints; ++j)
    {
        m_axisNames.push_back("joint_" + std::to_string(j));
    }

    // Override with user-supplied names if present.
    if (config.check<std::vector<std::string>>("joint_names"))
    {
        const auto& names = config.find("joint_names").as<std::vector<std::string>>();
        if (static_cast<int>(names.size()) == m_njoints)
        {
            m_axisNames = names;
        }
    }

    return true;
}

bool MyDevice::close()
{
    m_axisNames.clear();
    m_njoints = 0;
    m_gain = 1.0;
    return true;
}

bool MyDevice::getAxes(int* ax)
{
    if (!ax)
    {
        return false;
    }
    *ax = m_njoints;
    return true;
}

bool MyDevice::getAxisName(int axis, std::string& name)
{
    if (axis < 0 || axis >= m_njoints)
    {
        return false;
    }
    name = m_axisNames[static_cast<std::size_t>(axis)];
    return true;
}

bool MyDevice::getJointType(int axis, dinrail::JointType& type)
{
    if (axis < 0 || axis >= m_njoints)
    {
        return false;
    }
    type = dinrail::JointType::REVOLUTE;
    return true;
}

bool MyDevice::setGain(double gain)
{
    if (gain <= 0.0)
    {
        return false;
    }

    m_gain = gain;
    return true;
}

bool MyDevice::getGain(double& gain)
{
    gain = m_gain;
    return true;
}
