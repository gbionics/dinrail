// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpAxisInfoAdapter.h>

namespace dinrail
{

YarpAxisInfoAdapter::YarpAxisInfoAdapter(yarp::dev::PolyDriver* yarpDevice)
{
    if (yarpDevice && yarpDevice->isValid())
    {
        yarpDevice->view(m_yarpInterface);
    }
}

bool YarpAxisInfoAdapter::getAxes(int* ax)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getAxes(ax);
}

bool YarpAxisInfoAdapter::getAxisName(int axis, std::string& name)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getAxisName(axis, name);
}

bool YarpAxisInfoAdapter::getJointType(int axis, JointType& type)
{
    if (!m_yarpInterface)
    {
        return false;
    }

    yarp::dev::JointTypeEnum yarpType = yarp::dev::VOCAB_JOINTTYPE_UNKNOWN;
    if (!m_yarpInterface->getJointType(axis, yarpType))
    {
        return false;
    }

    switch (yarpType)
    {
    case yarp::dev::VOCAB_JOINTTYPE_REVOLUTE:
        type = JointType::REVOLUTE;
        return true;
    case yarp::dev::VOCAB_JOINTTYPE_PRISMATIC:
        type = JointType::PRISMATIC;
        return true;
    default:
        type = JointType::UNKNOWN;
        return true;
    }
}

} // namespace dinrail
