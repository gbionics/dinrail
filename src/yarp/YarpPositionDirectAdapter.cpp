// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "YarpPositionDirectAdapter.h"

namespace dinrail
{

YarpPositionDirectAdapter::YarpPositionDirectAdapter(yarp::dev::PolyDriver* yarpDevice)
{
    if (yarpDevice && yarpDevice->isValid())
    {
        yarpDevice->view(m_yarpInterface);
    }
}

bool YarpPositionDirectAdapter::getAxes(int *ax)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getAxes(ax);
}

bool YarpPositionDirectAdapter::setPosition(int j, double ref)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->setPosition(j, ref);
}

bool YarpPositionDirectAdapter::setPositions(const int n_joint, const int *joints, const double *refs)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->setPositions(n_joint, joints, refs);
}

bool YarpPositionDirectAdapter::setPositions(const double *refs)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->setPositions(refs);
}

bool YarpPositionDirectAdapter::getRefPosition(const int joint, double *ref)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getRefPosition(joint, ref);
}

bool YarpPositionDirectAdapter::getRefPositions(double *refs)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getRefPositions(refs);
}

bool YarpPositionDirectAdapter::getRefPositions(const int n_joint, const int *joints, double *refs)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getRefPositions(n_joint, joints, refs);
}

} // namespace dinrail
