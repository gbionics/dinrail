// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "YarpControlModeAdapter.h"

namespace dinrail
{

YarpControlModeAdapter::YarpControlModeAdapter(yarp::dev::PolyDriver* yarpDevice)
{
    if (yarpDevice && yarpDevice->isValid())
    {
        yarpDevice->view(m_yarpInterface);
    }
}

bool YarpControlModeAdapter::getControlMode(int j, int* mode)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getControlMode(j, mode);
}

bool YarpControlModeAdapter::getControlModes(int* modes)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getControlModes(modes);
}

bool YarpControlModeAdapter::getControlModes(const int n_joint, const int* joints, int* modes)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getControlModes(n_joint, joints, modes);
}

bool YarpControlModeAdapter::setControlMode(const int j, const int mode)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->setControlMode(j, mode);
}

bool YarpControlModeAdapter::setControlModes(const int n_joint, const int* joints, int* modes)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->setControlModes(n_joint, joints, modes);
}

bool YarpControlModeAdapter::setControlModes(int* modes)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->setControlModes(modes);
}

} // namespace dinrail
