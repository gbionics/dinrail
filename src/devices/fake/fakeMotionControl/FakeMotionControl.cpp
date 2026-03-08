// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "FakeMotionControl.h"
#include <dinrail/Constants.h>

// sharedlibpp class factory API
#include <sharedlibpp/SharedLibraryClassApi.h>

namespace dinrail
{

// Register this device with the plugin system
// The factory name must match what's returned by
// getSharedlibppFactoryNameFromDeviceName("fakeMotionControl")
SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_device_fakeMotionControl,
                               dinrail::FakeMotionControl,
                               dinrail::IDevice);

bool FakeMotionControl::open(const Parameters& config)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Get number of joints from config (default to 1)
    m_njoints = 1;

    // Check if GENERAL group exists and read Joints parameter
    const Parameters& general = config.findGroup("GENERAL");
    if (!general.isNull())
    {
        if (general.find("Joints").isInt())
        {
            m_njoints = general.find("Joints").as<int>();
        }
    }

    // Initialize internal state
    m_encoderPositions.resize(m_njoints, 0.0);
    m_refPositions.resize(m_njoints, 0.0);
    m_controlModes.resize(m_njoints, VOCAB_CM_POSITION);

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

// IPositionDirect interface
bool FakeMotionControl::getAxes(int* ax)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    *ax = m_njoints;
    return true;
}

bool FakeMotionControl::setPosition(int j, double ref)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j < 0 || j >= m_njoints)
        return false;

    m_refPositions[j] = ref;
    // In a fake device, we immediately "move" to the reference position
    m_encoderPositions[j] = ref;
    return true;
}

bool FakeMotionControl::setPositions(int n_joint, const int* joints, const double* refs)
{
    if (!joints || !refs)
        return false;

    for (int i = 0; i < n_joint; i++)
    {
        if (!setPosition(joints[i], refs[i]))
            return false;
    }
    return true;
}

bool FakeMotionControl::setPositions(const double* refs)
{
    if (!refs)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        m_refPositions[j] = refs[j];
        m_encoderPositions[j] = refs[j];
    }
    return true;
}

bool FakeMotionControl::getRefPosition(int joint, double* ref)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (joint < 0 || joint >= m_njoints || !ref)
        return false;

    *ref = m_refPositions[joint];
    return true;
}

bool FakeMotionControl::getRefPositions(double* refs)
{
    if (!refs)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        refs[j] = m_refPositions[j];
    }
    return true;
}

bool FakeMotionControl::getRefPositions(int n_joint, const int* joints, double* refs)
{
    if (!joints || !refs)
        return false;

    for (int i = 0; i < n_joint; i++)
    {
        if (!getRefPosition(joints[i], &refs[i]))
            return false;
    }
    return true;
}

// IEncoders interface
bool FakeMotionControl::resetEncoder(int j)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j < 0 || j >= m_njoints)
        return false;

    m_encoderPositions[j] = 0.0;
    return true;
}

bool FakeMotionControl::resetEncoders()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        m_encoderPositions[j] = 0.0;
    }
    return true;
}

bool FakeMotionControl::setEncoder(int j, double val)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j < 0 || j >= m_njoints)
        return false;

    m_encoderPositions[j] = val;
    return true;
}

bool FakeMotionControl::setEncoders(const double* vals)
{
    if (!vals)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        m_encoderPositions[j] = vals[j];
    }
    return true;
}

bool FakeMotionControl::getEncoder(int j, double* v)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j < 0 || j >= m_njoints || !v)
        return false;

    *v = m_encoderPositions[j];
    return true;
}

bool FakeMotionControl::getEncoders(double* encs)
{
    if (!encs)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        encs[j] = m_encoderPositions[j];
    }
    return true;
}

bool FakeMotionControl::getEncoderSpeed(int j, double* sp)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j < 0 || j >= m_njoints || !sp)
        return false;

    // Fake device always returns 0 speed
    *sp = 0.0;
    return true;
}

bool FakeMotionControl::getEncoderSpeeds(double* spds)
{
    if (!spds)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        spds[j] = 0.0;
    }
    return true;
}

bool FakeMotionControl::getEncoderAcceleration(int j, double* spds)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j < 0 || j >= m_njoints || !spds)
        return false;

    // Fake device always returns 0 acceleration
    *spds = 0.0;
    return true;
}

bool FakeMotionControl::getEncoderAccelerations(double* accs)
{
    if (!accs)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        accs[j] = 0.0;
    }
    return true;
}

// IControlMode interface
bool FakeMotionControl::getControlMode(int j, int* mode)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j < 0 || j >= m_njoints || !mode)
        return false;

    *mode = m_controlModes[j];
    return true;
}

bool FakeMotionControl::getControlModes(int* modes)
{
    if (!modes)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        modes[j] = m_controlModes[j];
    }
    return true;
}

bool FakeMotionControl::getControlModes(int n_joint, const int* joints, int* modes)
{
    if (!joints || !modes)
        return false;

    for (int i = 0; i < n_joint; i++)
    {
        if (!getControlMode(joints[i], &modes[i]))
            return false;
    }
    return true;
}

bool FakeMotionControl::setControlMode(int j, int mode)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (j < 0 || j >= m_njoints)
        return false;

    m_controlModes[j] = mode;
    return true;
}

bool FakeMotionControl::setControlModes(int n_joint, const int* joints, int* modes)
{
    if (!joints || !modes)
        return false;

    for (int i = 0; i < n_joint; i++)
    {
        if (!setControlMode(joints[i], modes[i]))
            return false;
    }
    return true;
}

bool FakeMotionControl::setControlModes(int* modes)
{
    if (!modes)
        return false;

    std::lock_guard<std::mutex> lock(m_mutex);
    for (int j = 0; j < m_njoints; j++)
    {
        m_controlModes[j] = modes[j];
    }
    return true;
}

} // namespace dinrail
