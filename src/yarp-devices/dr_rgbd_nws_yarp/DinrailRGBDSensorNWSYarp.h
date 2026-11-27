// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_DINRAILRGBDSENSORNWSYARP_H
#define DINRAIL_DINRAILRGBDSENSORNWSYARP_H

#include <dinrail/Device.h>
#include <dinrail/ICameraControls.h>
#include <dinrail/IRGBDSensor.h>
#include <dinrail/YarpRGBDSensorAdapters.h>

#include <dinrail/yarp_proto/framegrabber/DepthVisualParams_Responder.h>
#include <dinrail/yarp_proto/framegrabber/FrameGrabberControls_Responder.h>
#include <dinrail/yarp_proto/framegrabber/RgbVisualParams_Responder.h>

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/WrapperSingle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/os/Port.h>
#include <yarp/os/PortReader.h>
#include <yarp/sig/Image.h>

#include <cstdint>
#include <mutex>
#include <string>

/**
 * @brief Network wrapper server of a dinrail::IRGBDSensor.
 *
 * The device publishes the images of an RGBD sensor on YARP ports, and serves its parameters on
 * an RPC port, with the same protocol of the YARP rgbdSensor_nws_yarp, so that standard YARP
 * clients (RGBDSensorClient, yarpview, ...) and dr_rgbd_nwc_yarp can be connected to it.
 *
 * The sensor is either:
 * - the dinrail device named by the @c subdevice parameter, opened by this device with the other
 *   parameters of its configuration;
 * - a YARP device attached with the @c attach action, which wraps a dinrail device implementing
 *   dinrail::IRGBDSensor (dinrail::IWrappedDinrailDevice, for example dr_device_yarp), implements
 *   dinrail::IRGBDSensor or, through an adapter, yarp::dev::IRGBDSensor.
 *
 * A periodic thread checks the stamp of the latest images with the non-blocking
 * dinrail::IRGBDSensor::getImagesStamp(), reads them only when their sequence number changes and
 * publishes them, with their stamps as envelopes. As the getters never wait for the camera, the
 * period only bounds the latency added by the wrapper, and it does not need to match the framerate
 * of the camera. Sensors that do not stream color images are supported, only their depth is
 * published.
 *
 * See README.md for the parameters.
 */
class DinrailRGBDSensorNWSYarp : public yarp::dev::DeviceDriver,
                                 public yarp::dev::WrapperSingle,
                                 public yarp::os::PeriodicThread,
                                 public yarp::os::PortReader
{
public:
    DinrailRGBDSensorNWSYarp();
    ~DinrailRGBDSensorNWSYarp() override;

    // yarp::dev::DeviceDriver
    bool open(yarp::os::Searchable& config) override;
    bool close() override;

    // yarp::dev::WrapperSingle
    bool attach(yarp::dev::PolyDriver* driver) override;
    bool detach() override;

    // yarp::os::PeriodicThread
    void run() override;

    // yarp::os::PortReader, serves the RPC port.
    bool read(yarp::os::ConnectionReader& connection) override;

private:
    bool start(dinrail::IRGBDSensor& sensor, dinrail::ICameraControls* controls);
    void stopPublishing();
    bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& response);
    bool respondRgbdSensor(const yarp::os::Bottle& command, yarp::os::Bottle& response);

    std::string m_name;
    double m_period{0.005};

    dinrail::Device m_subdevice;
    dinrail::InterfaceAdapter<dinrail::IRGBDSensor, yarp::dev::IRGBDSensor> m_sensorFromYarp;

    // Interfaces used by the thread and by the RPC port, protected by m_mutex.
    std::mutex m_mutex;
    dinrail::IRGBDSensor* m_sensor{nullptr};
    dinrail::InterfaceAdapter<yarp::dev::IRGBDSensor, dinrail::IRGBDSensor> m_yarpSensor;
    dinrail::InterfaceAdapter<yarp::dev::IFrameGrabberControls, dinrail::ICameraControls>
        m_yarpControls;
    bool m_hasControls{false};
    dinrail::yarp_proto::framegrabber::RgbVisualParams_Responder m_rgbResponder;
    dinrail::yarp_proto::framegrabber::DepthVisualParams_Responder m_depthResponder;
    dinrail::yarp_proto::framegrabber::FrameGrabberControls_Responder m_controlsResponder;

    yarp::os::BufferedPort<yarp::sig::FlexImage> m_rgbPort;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelFloat>> m_depthPort;
    yarp::os::Port m_rpcPort;

    dinrail::Image m_rgb;
    dinrail::DepthImage m_depth;
    std::int32_t m_lastSequenceNumber{-1};
    bool m_hasPublished{false};
    bool m_depthOnly{false};
};

#endif // DINRAIL_DINRAILRGBDSENSORNWSYARP_H
