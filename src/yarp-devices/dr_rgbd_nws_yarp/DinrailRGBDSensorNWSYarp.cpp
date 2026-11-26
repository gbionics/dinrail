// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "DinrailRGBDSensorNWSYarp.h"

#include <dinrail/IWrappedDinrailDevice.h>
#include <dinrail/SpdlogHelpers.h>
#include <dinrail/YarpPropertyConverter.h>
#include <dinrail/YarpStampConverter.h>
#include <dinrail/yarp_proto/framegrabber/CameraVocabs.h>

#include <yarp/dev/GenericVocabs.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/ConnectionReader.h>
#include <yarp/os/ConnectionWriter.h>
#include <yarp/os/Property.h>
#include <yarp/sig/Matrix.h>

namespace
{

// Version of the protocol of the YARP rgbdSensor_nws_yarp, checked by RGBDSensorClient.
constexpr int protocolVersionMajor = 1;
constexpr int protocolVersionMinor = 0;

spdlog::logger& logger()
{
    static const auto instance = [] {
        auto created = dinrail::createOrGetLogger("dinrail.rgbd_nws_yarp");
        return created ? created : spdlog::default_logger();
    }();
    return *instance;
}

} // namespace

DinrailRGBDSensorNWSYarp::DinrailRGBDSensorNWSYarp()
    : yarp::os::PeriodicThread(0.005)
{
}

DinrailRGBDSensorNWSYarp::~DinrailRGBDSensorNWSYarp()
{
    close();
}

bool DinrailRGBDSensorNWSYarp::open(yarp::os::Searchable& config)
{
    m_name = config.check("name", yarp::os::Value("")).asString();
    if (m_name.empty() || m_name.front() != '/')
    {
        logger().error("The parameter 'name' is mandatory and must start with '/'.");
        return false;
    }
    m_period = config.check("period", yarp::os::Value(0.005)).asFloat64();
    if (m_period <= 0.0)
    {
        logger().error("The parameter 'period' must be positive.");
        return false;
    }

    if (!m_rgbPort.open(m_name + "/rgbImage:o") || !m_depthPort.open(m_name + "/depthImage:o"))
    {
        logger().error("Unable to open the image ports with prefix {}.", m_name);
        close();
        return false;
    }
    m_rpcPort.setReader(*this);
    if (!m_rpcPort.open(m_name + "/rpc:i"))
    {
        logger().error("Unable to open the port {}/rpc:i.", m_name);
        close();
        return false;
    }

    if (!config.check("subdevice"))
    {
        // The sensor is attached later.
        return true;
    }

    yarp::os::Property subdeviceConfig;
    subdeviceConfig.fromString(config.toString());
    subdeviceConfig.unput("name");
    subdeviceConfig.unput("period");
    subdeviceConfig.unput("subdevice");
    subdeviceConfig.unput("device");
    // Added by yarprobotinterface and dinrail-runner to every device.
    subdeviceConfig.unput("id");
    subdeviceConfig.unput("robotName");
    dinrail::Parameters parameters
        = dinrail::YarpPropertyConverter::toDinrailParameters(subdeviceConfig);
    parameters.put("device", config.find("subdevice").asString());

    dinrail::IRGBDSensor* sensor = nullptr;
    dinrail::ICameraControls* controls = nullptr;
    if (!m_subdevice.open(parameters) || !m_subdevice.view(sensor) || sensor == nullptr)
    {
        logger().error("Unable to open the subdevice '{}' with the dinrail::IRGBDSensor "
                       "interface.",
                       config.find("subdevice").asString());
        close();
        return false;
    }
    m_subdevice.view(controls);
    if (!start(*sensor, controls))
    {
        close();
        return false;
    }
    return true;
}

bool DinrailRGBDSensorNWSYarp::close()
{
    detach();
    if (m_subdevice.isValid())
    {
        m_subdevice.close();
    }
    m_rpcPort.close();
    m_rgbPort.close();
    m_depthPort.close();
    return true;
}

bool DinrailRGBDSensorNWSYarp::attach(yarp::dev::PolyDriver* driver)
{
    if (driver == nullptr || !driver->isValid())
    {
        logger().error("Unable to attach an invalid device.");
        return false;
    }

    dinrail::IRGBDSensor* sensor = nullptr;
    dinrail::ICameraControls* controls = nullptr;
    dinrail::IWrappedDinrailDevice* wrapped = nullptr;
    if (driver->view(wrapped) && wrapped != nullptr)
    {
        // Use the native interfaces of the dinrail device opened by the YARP device.
        wrapped->wrappedDinrailDevice().view(sensor);
        wrapped->wrappedDinrailDevice().view(controls);
    } else
    {
        driver->view(sensor);
        driver->view(controls);
    }
    if (sensor == nullptr)
    {
        // Standard YARP RGBD sensors are supported through an adapter.
        yarp::dev::IRGBDSensor* yarpSensor = nullptr;
        if (!driver->view(yarpSensor) || yarpSensor == nullptr)
        {
            logger().error("The attached device provides neither dinrail::IRGBDSensor nor "
                           "yarp::dev::IRGBDSensor.");
            return false;
        }
        m_sensorFromYarp.bind(*yarpSensor);
        sensor = &m_sensorFromYarp;
    }
    return start(*sensor, controls);
}

bool DinrailRGBDSensorNWSYarp::start(dinrail::IRGBDSensor& sensor,
                                     dinrail::ICameraControls* controls)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sensor = &sensor;
        m_yarpSensor.bind(sensor);
        m_rgbResponder.configure(&m_yarpSensor);
        m_depthResponder.configure(&m_yarpSensor);
        m_hasControls = controls != nullptr;
        if (controls != nullptr)
        {
            m_yarpControls.bind(*controls);
            m_controlsResponder.configure(&m_yarpControls);
        }
        m_lastSequenceNumber = -1;
        m_hasPublished = false;
        m_depthOnly = false;
    }

    setPeriod(m_period);
    if (!yarp::os::PeriodicThread::start())
    {
        logger().error("Unable to start the publishing thread.");
        stopPublishing();
        return false;
    }
    logger().info("Publishing the RGBD sensor on {}/rgbImage:o and {}/depthImage:o.",
                  m_name,
                  m_name);
    return true;
}

bool DinrailRGBDSensorNWSYarp::detach()
{
    stopPublishing();
    return true;
}

void DinrailRGBDSensorNWSYarp::stopPublishing()
{
    if (isRunning())
    {
        stop();
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sensor = nullptr;
    m_hasControls = false;
    m_yarpSensor.unbind();
    m_yarpControls.unbind();
    m_sensorFromYarp.unbind();
}

void DinrailRGBDSensorNWSYarp::run()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_sensor == nullptr)
    {
        return;
    }
    // Copy the images only when they changed, if the sensor provides their stamp.
    dinrail::Stamp stamp;
    if (m_hasPublished && m_sensor->getImagesStamp(stamp).ok()
        && stamp.sequenceNumber == m_lastSequenceNumber)
    {
        return;
    }
    dinrail::Status status;
    if (!m_depthOnly)
    {
        status = m_sensor->getImages(m_rgb, m_depth);
        if (status.code() == dinrail::StatusCode::FailedPrecondition)
        {
            logger().info("The sensor does not stream color images, only the depth is published.");
            m_depthOnly = true;
        }
    }
    if (m_depthOnly)
    {
        status = m_sensor->getDepthImage(m_depth);
    }
    if (!status.ok() || (m_hasPublished && m_depth.stamp.sequenceNumber == m_lastSequenceNumber))
    {
        return;
    }
    m_lastSequenceNumber = m_depth.stamp.sequenceNumber;
    m_hasPublished = true;

    if (!m_depthOnly && m_rgbPort.getOutputCount() > 0)
    {
        yarp::sig::FlexImage& image = m_rgbPort.prepare();
        if (dinrail::toYarpImage(m_rgb, image))
        {
            yarp::os::Stamp stamp = dinrail::toYarpStamp(m_rgb.stamp);
            m_rgbPort.setEnvelope(stamp);
            m_rgbPort.write();
        } else
        {
            m_rgbPort.unprepare();
        }
    }
    if (m_depthPort.getOutputCount() > 0)
    {
        dinrail::toYarpImage(m_depth, m_depthPort.prepare());
        yarp::os::Stamp stamp = dinrail::toYarpStamp(m_depth.stamp);
        m_depthPort.setEnvelope(stamp);
        m_depthPort.write();
    }
}

bool DinrailRGBDSensorNWSYarp::read(yarp::os::ConnectionReader& connection)
{
    yarp::os::Bottle command;
    yarp::os::Bottle response;
    if (!command.read(connection))
    {
        return false;
    }
    respond(command, response);
    if (yarp::os::ConnectionWriter* writer = connection.getWriter(); writer != nullptr)
    {
        response.write(*writer);
    }
    return true;
}

bool DinrailRGBDSensorNWSYarp::respond(const yarp::os::Bottle& command, yarp::os::Bottle& response)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    response.clear();
    if (m_sensor == nullptr)
    {
        response.addVocab32(VOCAB_FAILED);
        return false;
    }

    switch (command.get(0).asVocab32())
    {
    case VOCAB_RGB_VISUAL_PARAMS:
        return m_rgbResponder.respond(command, response);
    case VOCAB_DEPTH_VISUAL_PARAMS:
        return m_depthResponder.respond(command, response);
    case VOCAB_FRAMEGRABBER_CONTROL:
        if (!m_hasControls)
        {
            response.addVocab32(VOCAB_FAILED);
            return false;
        }
        return m_controlsResponder.respond(command, response);
    case VOCAB_RGBD_SENSOR:
        return respondRgbdSensor(command, response);
    default:
        logger().warn("Received a command for an unknown interface: {}", command.toString());
        response.addVocab32(VOCAB_FAILED);
        return false;
    }
}

bool DinrailRGBDSensorNWSYarp::respondRgbdSensor(const yarp::os::Bottle& command,
                                                 yarp::os::Bottle& response)
{
    if (command.get(1).asVocab32() != VOCAB_GET)
    {
        response.addVocab32(VOCAB_FAILED);
        return false;
    }

    const auto addHeader = [&response](yarp::conf::vocab32_t request) {
        response.addVocab32(VOCAB_RGBD_SENSOR);
        response.addVocab32(request);
        response.addVocab32(VOCAB_IS);
    };

    switch (command.get(2).asVocab32())
    {
    case VOCAB_EXTRINSIC_PARAM: {
        yarp::sig::Matrix extrinsics;
        yarp::os::Bottle serialized;
        if (!m_yarpSensor.getExtrinsicParam(extrinsics)
            || !yarp::os::Property::copyPortable(extrinsics, serialized))
        {
            response.addVocab32(VOCAB_FAILED);
            return false;
        }
        addHeader(VOCAB_EXTRINSIC_PARAM);
        // The matrix is sent as a single element, which is what RGBDSensorClient reads. The YARP
        // rgbdSensor_nws_yarp appends its elements instead, so RGBDSensorClient cannot read them.
        response.addList() = serialized;
        return true;
    }
    case VOCAB_ERROR_MSG:
        addHeader(VOCAB_ERROR_MSG);
        response.addString(m_yarpSensor.getLastErrorMsg());
        return true;
    case VOCAB_RGBD_PROTOCOL_VERSION:
        addHeader(VOCAB_RGBD_PROTOCOL_VERSION);
        response.addInt32(protocolVersionMajor);
        response.addInt32(protocolVersionMinor);
        return true;
    case VOCAB_STATUS:
        addHeader(VOCAB_STATUS);
        response.addInt32(m_yarpSensor.getSensorStatus());
        return true;
    default:
        response.addVocab32(VOCAB_FAILED);
        return false;
    }
}
