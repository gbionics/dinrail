// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPAXISINFOTRANSLATION_H
#define DINRAIL_YARPAXISINFOTRANSLATION_H

#include <dinrail/IAxisInfo.h>
#include <dinrail/IInterfaceTranslation.h>

#include <yarp/dev/IAxisInfo.h>

namespace dinrail
{

/** Translate yarp::dev::IAxisInfo calls to dinrail::IAxisInfo. */
class YarpAxisInfoTranslation final : public IInterfaceTranslation, public IAxisInfo
{
public:
    explicit YarpAxisInfoTranslation(yarp::dev::IAxisInfo& yarpInterface);

    void* getInterface() override;

    bool getAxes(int* ax) override;
    bool getAxisName(int axis, std::string& name) override;
    bool getJointType(int axis, JointType& type) override;

private:
    yarp::dev::IAxisInfo& m_yarpInterface;
};

} // namespace dinrail

#endif // DINRAIL_YARPAXISINFOTRANSLATION_H
