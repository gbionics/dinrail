// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpAxisInfoTranslation.h>

namespace dinrail
{

YarpAxisInfoTranslation::YarpAxisInfoTranslation(yarp::dev::IAxisInfo& yarpInterface)
    : m_yarpInterface(yarpInterface)
{
}

void* YarpAxisInfoTranslation::getInterface()
{
    return static_cast<dinrail::IAxisInfo*>(this);
}

bool YarpAxisInfoTranslation::getAxes(int* ax)
{
    return m_yarpInterface.getAxes(ax);
}

bool YarpAxisInfoTranslation::getAxisName(int axis, std::string& name)
{
    return m_yarpInterface.getAxisName(axis, name);
}

bool YarpAxisInfoTranslation::getJointType(int axis, JointType& type)
{
    yarp::dev::JointTypeEnum yarpType = yarp::dev::VOCAB_JOINTTYPE_UNKNOWN;
    if (!m_yarpInterface.getJointType(axis, yarpType))
    {
        return false;
    }

    switch (yarpType)
    {
    case yarp::dev::VOCAB_JOINTTYPE_REVOLUTE:
        type = JointType::REVOLUTE;
        break;
    case yarp::dev::VOCAB_JOINTTYPE_PRISMATIC:
        type = JointType::PRISMATIC;
        break;
    default:
        type = JointType::UNKNOWN;
        break;
    }

    return true;
}

} // namespace dinrail
