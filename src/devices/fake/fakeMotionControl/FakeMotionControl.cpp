// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "FakeMotionControl.h"

#include <sharedlibpp/SharedLibraryClassApi.h>

#include <algorithm>
#include <cctype>

namespace dinrail
{

namespace
{
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

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_device_fakeMotionControl,
                               dinrail::FakeMotionControl,
                               dinrail::IDevice);

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

IDevice* FakeMotionControl::allocateInstance() const
{
    return new FakeMotionControl();
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
    if (!m_opened || axis < 0 || axis >= m_njoints)
    {
        return false;
    }

    type = m_jointTypes[static_cast<std::size_t>(axis)];
    return true;
}

} // namespace dinrail
