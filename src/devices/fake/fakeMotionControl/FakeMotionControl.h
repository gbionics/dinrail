// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_FAKEMOTIONCONTROL_H
#define DINRAIL_FAKEMOTIONCONTROL_H

#include <dinrail/IAxisInfo.h>
#include <dinrail/IDevice.h>

#include <mutex>
#include <string>
#include <vector>

namespace dinrail
{

class FakeMotionControl : public IDevice, public IAxisInfo
{
public:
    FakeMotionControl() = default;
    ~FakeMotionControl() override = default;

    bool open(const Parameters& config) override;
    bool close() override;
    IDevice* allocateInstance() const override;

    bool getAxes(int* ax) override;
    bool getAxisName(int axis, std::string& name) override;
    bool getJointType(int axis, JointType& type) override;

private:
    std::mutex m_mutex;
    int m_njoints{0};
    bool m_opened{false};
    std::vector<std::string> m_axisNames;
    std::vector<JointType> m_jointTypes;
};

} // namespace dinrail

#endif // DINRAIL_FAKEMOTIONCONTROL_H
