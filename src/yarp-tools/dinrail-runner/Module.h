/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef YARP_YARPROBOTINTERFACE_MODULE_H
#define YARP_YARPROBOTINTERFACE_MODULE_H

#include <yarp/os/RFModule.h>

namespace yarprobotinterface
{

class Module : public yarp::os::RFModule
{
public:
    explicit Module();
    virtual ~Module();

    // yarp::os::RFModule
    double getPeriod() override;
    bool updateModule() override;
    bool close() override;
    bool interruptModule() override;
    bool configure(yarp::os::ResourceFinder &rf) override;

private:
    class Private;
    Private * const mPriv;
}; // class Module

} // namespace yarprobotinterface

#endif // YARP_YARPROBOTINTERFACE_MODULE_H
