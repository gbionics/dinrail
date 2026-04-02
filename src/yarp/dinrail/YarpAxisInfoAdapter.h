// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPAXISINFOADAPTER_H
#define DINRAIL_YARPAXISINFOADAPTER_H

#include <dinrail/IAxisInfo.h>

#include <yarp/dev/IAxisInfo.h>
#include <yarp/dev/PolyDriver.h>

namespace dinrail
{

class YarpAxisInfoAdapter : public IAxisInfo
{
public:
    explicit YarpAxisInfoAdapter(yarp::dev::PolyDriver* yarpDevice);

    bool getAxes(int* ax) override;
    bool getAxisName(int axis, std::string& name) override;
    bool getJointType(int axis, JointType& type) override;

private:
    yarp::dev::IAxisInfo* m_yarpInterface{nullptr};
};

} // namespace dinrail

#endif // DINRAIL_YARPAXISINFOADAPTER_H
