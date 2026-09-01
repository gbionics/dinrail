// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_FAKEMOTIONCONTROL_H
#define DINRAIL_FAKEMOTIONCONTROL_H

#include <dinrail/IAxisInfo.h>
#include <dinrail/IDevice.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/IJointFault.h>
#include <dinrail/IMotor.h>
#include <dinrail/IMotorEncoders.h>
#include <dinrail/IPreciselyTimed.h>

#include <mutex>
#include <string>
#include <vector>

namespace dinrail
{

class FakeMotionControl : public IDevice,
                          public IAxisInfo,
                          public IEncoders,
                          public IEncodersSimulation,
                          public IImpedanceAllSetPointsControl,
                          public IJointFault,
                          public IJointFaultSimulation,
                          public IMotor,
                          public IMotorSimulation,
                          public IMotorEncoders,
                          public IMotorEncodersSimulation,
                          public IPreciselyTimed,
                          public IPreciselyTimedSimulation
{
public:
    FakeMotionControl() = default;
    ~FakeMotionControl() override = default;

    bool open(const Parameters& config) override;
    bool close() override;

    bool getAxes(int* ax) override;
    bool getAxisName(int axis, std::string& name) override;
    bool getJointType(int axis, JointType& type) override;

    bool getEncoder(int j, double* value) override;
    bool getEncoders(VectorProxy<double>::Ref values) override;
    bool getEncoderTimed(int j, double* value, double* timestamp) override;
    bool
    getEncodersTimed(VectorProxy<double>::Ref values, VectorProxy<double>::Ref timestamps) override;
    bool getEncoderSpeed(int j, double* speed) override;
    bool getEncoderSpeeds(VectorProxy<double>::Ref speeds) override;
    bool getEncoderAcceleration(int j, double* acceleration) override;
    bool getEncoderAccelerations(VectorProxy<double>::Ref accelerations) override;

    bool getNumberOfMotors(int* num) override;
    bool getTemperature(int m, double* value) override;
    bool getTemperatures(VectorProxy<double>::Ref values) override;
    bool getTemperatureLimit(int m, double* temperature) override;
    bool setTemperatureLimit(int m, double temperature) override;
    bool getGearboxRatio(int m, double* value) override;
    bool setGearboxRatio(int m, double value) override;

    bool setTemperature(int m, double value) override;
    bool setTemperatures(const VectorProxy<const double>::Ref values) override;

    bool getNumberOfMotorEncoders(int* num) override;
    bool getMotorEncoderCountsPerRevolution(int m, double* countsPerRevolution) override;
    bool getMotorEncoder(int m, double* value) override;
    bool getMotorEncoders(VectorProxy<double>::Ref values) override;
    bool getMotorEncoderTimed(int m, double* value, double* timestamp) override;
    bool getMotorEncodersTimed(VectorProxy<double>::Ref values,
                               VectorProxy<double>::Ref timestamps) override;
    bool getMotorEncoderSpeed(int m, double* speed) override;
    bool getMotorEncoderSpeeds(VectorProxy<double>::Ref speeds) override;
    bool getMotorEncoderAcceleration(int m, double* acceleration) override;
    bool getMotorEncoderAccelerations(VectorProxy<double>::Ref accelerations) override;

    bool setMotorEncoder(int m, double value) override;
    bool setMotorEncoders(const VectorProxy<const double>::Ref values) override;
    bool setMotorEncoderTimed(int m, double value, double timestamp) override;
    bool setMotorEncodersTimed(const VectorProxy<const double>::Ref values,
                               const VectorProxy<const double>::Ref timestamps) override;
    bool setMotorEncoderSpeed(int m, double speed) override;
    bool setMotorEncoderSpeeds(const VectorProxy<const double>::Ref speeds) override;
    bool setMotorEncoderAcceleration(int m, double acceleration) override;
    bool setMotorEncoderAccelerations(const VectorProxy<const double>::Ref accelerations) override;

    bool getLastJointFault(int j, int& fault, std::string& message) override;
    bool setLastJointFault(int j, int fault, const std::string& message) override;

    bool setEncoder(int j, double value) override;
    bool setEncoders(const VectorProxy<const double>::Ref values) override;
    bool setEncoderTimed(int j, double value, double timestamp) override;
    bool setEncodersTimed(const VectorProxy<const double>::Ref values,
                          const VectorProxy<const double>::Ref timestamps) override;
    bool setEncoderSpeed(int j, double speed) override;
    bool setEncoderSpeeds(const VectorProxy<const double>::Ref speeds) override;
    bool setEncoderAcceleration(int j, double acceleration) override;
    bool setEncoderAccelerations(const VectorProxy<const double>::Ref accelerations) override;

    Stamp getLastInputStamp() override;
    void setLastInputStamp(const Stamp& stamp) override;

    bool setSetPoint(
        int j, double pos, double vel, double torque, double stiffness, double damping) override;
    bool setSetPoints(const VectorProxy<const int>::Ref jointIndeces,
                      const VectorProxy<const double>::Ref pos,
                      const VectorProxy<const double>::Ref vel,
                      const VectorProxy<const double>::Ref torque,
                      const VectorProxy<const double>::Ref stiffness,
                      const VectorProxy<const double>::Ref damping) override;
    bool setSetPoints(const VectorProxy<const double>::Ref pos,
                      const VectorProxy<const double>::Ref vel,
                      const VectorProxy<const double>::Ref torque,
                      const VectorProxy<const double>::Ref stiffness,
                      const VectorProxy<const double>::Ref damping) override;
    bool
    getSetPoint(int j, double& pos, double& vel, double& torque, double& stiffness, double& damping)
        override;
    bool getSetPoints(const VectorProxy<const int>::Ref jointIndeces,
                      VectorProxy<double>::Ref pos,
                      VectorProxy<double>::Ref vel,
                      VectorProxy<double>::Ref torque,
                      VectorProxy<double>::Ref stiffness,
                      VectorProxy<double>::Ref damping) override;
    bool getSetPoints(VectorProxy<double>::Ref pos,
                      VectorProxy<double>::Ref vel,
                      VectorProxy<double>::Ref torque,
                      VectorProxy<double>::Ref stiffness,
                      VectorProxy<double>::Ref damping) override;

private:
    std::mutex m_mutex;
    int m_njoints{0};
    bool m_opened{false};
    Stamp m_lastInputStamp;
    std::vector<std::string> m_axisNames;
    std::vector<JointType> m_jointTypes;
    std::vector<double> m_encoderPositions;
    std::vector<double> m_encoderTimestamps;
    std::vector<double> m_encoderSpeeds;
    std::vector<double> m_encoderAccelerations;
    std::vector<double> m_motorTemperatures;
    std::vector<double> m_motorTemperatureLimits;
    std::vector<double> m_motorGearboxRatios;
    std::vector<double> m_motorEncoderCountsPerRevolution;
    std::vector<int> m_jointFaults;
    std::vector<std::string> m_jointFaultMessages;
    std::vector<double> m_posSetpoints;
    std::vector<double> m_velSetpoints;
    std::vector<double> m_torqueSetpoints;
    std::vector<double> m_stiffnessSetpoints;
    std::vector<double> m_dampingSetpoints;
};

} // namespace dinrail

#endif // DINRAIL_FAKEMOTIONCONTROL_H
