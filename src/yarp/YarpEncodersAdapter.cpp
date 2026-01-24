// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "YarpEncodersAdapter.h"

namespace dinrail
{

YarpEncodersAdapter::YarpEncodersAdapter(yarp::dev::PolyDriver* yarpDevice)
{
    if (yarpDevice && yarpDevice->isValid())
    {
        yarpDevice->view(m_yarpInterface);
    }
}

bool YarpEncodersAdapter::getAxes(int *ax)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getAxes(ax);
}

bool YarpEncodersAdapter::resetEncoder(int j)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->resetEncoder(j);
}

bool YarpEncodersAdapter::resetEncoders()
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->resetEncoders();
}

bool YarpEncodersAdapter::setEncoder(int j, double val)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->setEncoder(j, val);
}

bool YarpEncodersAdapter::setEncoders(const double *vals)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->setEncoders(vals);
}

bool YarpEncodersAdapter::getEncoder(int j, double *v)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getEncoder(j, v);
}

bool YarpEncodersAdapter::getEncoders(double *encs)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getEncoders(encs);
}

bool YarpEncodersAdapter::getEncoderSpeed(int j, double *sp)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getEncoderSpeed(j, sp);
}

bool YarpEncodersAdapter::getEncoderSpeeds(double *spds)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getEncoderSpeeds(spds);
}

bool YarpEncodersAdapter::getEncoderAcceleration(int j, double *spds)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getEncoderAcceleration(j, spds);
}

bool YarpEncodersAdapter::getEncoderAccelerations(double *accs)
{
    if (!m_yarpInterface)
    {
        return false;
    }
    return m_yarpInterface->getEncoderAccelerations(accs);
}

} // namespace dinrail
