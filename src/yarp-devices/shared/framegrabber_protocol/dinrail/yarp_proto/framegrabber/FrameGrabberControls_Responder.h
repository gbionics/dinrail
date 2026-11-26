/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Copied from YARP (src/devices/messages/framegrabber_protocol, YARP 3.12), as the YARP library is
// not installed. The namespace has been changed to avoid clashes with the YARP plugins loaded in
// the same process, the protocol is unchanged.

#ifndef DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_FRAMEGRABBERCONTROLS_RESPONDER_H
#define DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_FRAMEGRABBERCONTROLS_RESPONDER_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/IFrameGrabberControls.h>

namespace dinrail::yarp_proto::framegrabber {

class FrameGrabberControls_Responder :
        public yarp::dev::DeviceResponder
{
private:
    yarp::dev::IFrameGrabberControls* fgCtrl;

public:
    FrameGrabberControls_Responder();
    ~FrameGrabberControls_Responder() override = default;

    bool configure(yarp::dev::IFrameGrabberControls* interface);
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& response) override;
};

} // namespace dinrail::yarp_proto::framegrabber

#endif // DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_FRAMEGRABBERCONTROLS_RESPONDER_H
