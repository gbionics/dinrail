// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IAXISINFO_H
#define DINRAIL_IAXISINFO_H

#include <string>

namespace dinrail
{

enum class JointType
{
    REVOLUTE = 1,
    PRISMATIC = 2,
    UNKNOWN = 0
};

class IAxisInfo
{
public:
    virtual ~IAxisInfo() = default;

    virtual bool getAxes(int* ax) = 0;
    virtual bool getAxisName(int axis, std::string& name) = 0;
    virtual bool getJointType(int axis, JointType& type) = 0;
};

} // namespace dinrail

#endif // DINRAIL_IAXISINFO_H
