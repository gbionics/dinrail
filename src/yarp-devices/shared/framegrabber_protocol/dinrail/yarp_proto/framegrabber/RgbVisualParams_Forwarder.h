/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Copied from YARP (src/devices/messages/framegrabber_protocol, YARP 3.12), as the YARP library is
// not installed. The namespace has been changed to avoid clashes with the YARP plugins loaded in
// the same process, the protocol is unchanged.

#ifndef DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_RGBVISUALPARAMS_FORWARDER_H
#define DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_RGBVISUALPARAMS_FORWARDER_H

#include <yarp/dev/IRgbVisualParams.h>
#include <yarp/os/Port.h>

namespace dinrail::yarp_proto::framegrabber {

class RgbVisualParams_Forwarder :
        public yarp::dev::IRgbVisualParams
{
protected:
    yarp::os::Port& m_port;

public:
    RgbVisualParams_Forwarder(yarp::os::Port& port);
    ~RgbVisualParams_Forwarder() override = default;

    int getRgbHeight() override;
    int getRgbWidth() override;
    bool getRgbSupportedConfigurations(yarp::sig::VectorOf<yarp::dev::CameraConfig>& configurations) override;
    bool getRgbResolution(int& width, int& height) override;
    bool setRgbResolution(int width, int height) override;
    bool getRgbFOV(double& horizontalFov, double& verticalFov) override;
    bool setRgbFOV(double horizontalFov, double verticalFov) override;
    bool getRgbIntrinsicParam(yarp::os::Property& intrinsic) override;
    bool getRgbMirroring(bool& mirror) override;
    bool setRgbMirroring(bool mirror) override;
};

} // namespace dinrail::yarp_proto::framegrabber

#endif // DINRAIL_YARP_FRAMEGRABBER_PROTOCOL_RGBVISUALPARAMS_FORWARDER_H
