// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_FAKEMOTIONCONTROL_H
#define DINRAIL_FAKEMOTIONCONTROL_H

#include <dinrail/IAxisInfo.h>
#include <dinrail/IDevice.h>
#include <dinrail/IImpedanceAllSetPointsControl.h>

#include <mutex>
#include <string>
#include <vector>

namespace dinrail
{

class FakeMotionControl : public IDevice, public IAxisInfo, public IImpedanceAllSetPointsControl
{
public:
    FakeMotionControl() = default;
    ~FakeMotionControl() override = default;

    bool open(const Parameters& config) override;
    bool close() override;

    bool getAxes(int* ax) override;
    bool getAxisName(int axis, std::string& name) override;
    bool getJointType(int axis, JointType& type) override;

    bool setSetPoint(int j,
                     double pos,
                     double vel,
                     double torque,
                     double stiffness,
                     double damping) override;
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
    bool getSetPoint(int j,
                     double& pos,
                     double& vel,
                     double& torque,
                     double& stiffness,
                     double& damping) override;
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
    std::vector<std::string> m_axisNames;
    std::vector<JointType> m_jointTypes;
    std::vector<double> m_posSetpoints;
    std::vector<double> m_velSetpoints;
    std::vector<double> m_torqueSetpoints;
    std::vector<double> m_stiffnessSetpoints;
    std::vector<double> m_dampingSetpoints;
};

} // namespace dinrail

#endif // DINRAIL_FAKEMOTIONCONTROL_H
