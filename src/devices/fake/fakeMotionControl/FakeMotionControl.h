// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_FAKEMOTIONCONTROL_H
#define DINRAIL_FAKEMOTIONCONTROL_H

#include <dinrail/IDevice.h>
#include <dinrail/IPositionDirect.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IControlMode.h>

#include <vector>
#include <mutex>

namespace dinrail
{

/**
 * @brief FakeMotionControl: A simple fake motion control device for testing.
 *
 * This device provides basic implementations of IPositionDirect, IEncoders, and IControlMode
 * interfaces without any actual hardware communication. It's useful for testing applications
 * that need motion control interfaces.
 */
class FakeMotionControl : public IDevice,
                          public IPositionDirect,
                          public IEncoders,
                          public IControlMode
{
private:
    std::mutex m_mutex;
    int m_njoints{0};
    std::vector<double> m_encoderPositions;
    std::vector<double> m_refPositions;
    std::vector<int> m_controlModes;
    bool m_opened{false};

public:
    FakeMotionControl() = default;
    ~FakeMotionControl() override = default;

    // IDevice interface
    bool open(const Property& config) override;
    bool close() override;
    IDevice* allocateInstance() const override;

    // IPositionDirect interface
    bool getAxes(int* ax) override;
    bool setPosition(int j, double ref) override;
    bool setPositions(int n_joint, const int* joints, const double* refs) override;
    bool setPositions(const double* refs) override;
    bool getRefPosition(int joint, double* ref) override;
    bool getRefPositions(double* refs) override;
    bool getRefPositions(int n_joint, const int* joints, double* refs) override;

    // IEncoders interface
    bool resetEncoder(int j) override;
    bool resetEncoders() override;
    bool setEncoder(int j, double val) override;
    bool setEncoders(const double* vals) override;
    bool getEncoder(int j, double* v) override;
    bool getEncoders(double* encs) override;
    bool getEncoderSpeed(int j, double* sp) override;
    bool getEncoderSpeeds(double* spds) override;
    bool getEncoderAcceleration(int j, double* spds) override;
    bool getEncoderAccelerations(double* accs) override;

    // IControlMode interface
    bool getControlMode(int j, int* mode) override;
    bool getControlModes(int* modes) override;
    bool getControlModes(int n_joint, const int* joints, int* modes) override;
    bool setControlMode(int j, int mode) override;
    bool setControlModes(int n_joint, const int* joints, int* modes) override;
    bool setControlModes(int* modes) override;
};

} // namespace dinrail

#endif // DINRAIL_FAKEMOTIONCONTROL_H
