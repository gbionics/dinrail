/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Copied from YARP (src/devices/messages/framegrabber_protocol, YARP 3.12), as the YARP library is
// not installed. The namespace has been changed to avoid clashes with the YARP plugins loaded in
// the same process, the protocol is unchanged.

#ifndef DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_DEPTHVISUALPARAMS_RESPONDER_H
#define DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_DEPTHVISUALPARAMS_RESPONDER_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/IDepthVisualParams.h>

namespace dinrail::yarp_proto::framegrabber
{

class DepthVisualParams_Responder : public yarp::dev::DeviceResponder
{
private:
    yarp::dev::IDepthVisualParams* iDepthVisual;

public:
    DepthVisualParams_Responder();
    ~DepthVisualParams_Responder() override = default;

    bool configure(yarp::dev::IDepthVisualParams* interface);
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& response) override;
};

} // namespace dinrail::yarp_proto::framegrabber

#endif // DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_DEPTHVISUALPARAMS_RESPONDER_H
