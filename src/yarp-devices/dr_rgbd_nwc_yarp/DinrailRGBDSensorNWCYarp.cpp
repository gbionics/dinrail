// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "DinrailRGBDSensorNWCYarp.h"

#include <dinrail/SpdlogHelpers.h>
#include <dinrail/YarpRGBDSensorAdapters.h>
#include <dinrail/yarp_proto/framegrabber/CameraVocabs.h>
#include <dinrail/yarp_proto/framegrabber/DepthVisualParams_Forwarder.h>
#include <dinrail/yarp_proto/framegrabber/RgbVisualParams_Forwarder.h>

#include <yarp/dev/GenericVocabs.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Network.h>
#include <yarp/os/Property.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Matrix.h>

namespace
{

// Version of the protocol of the YARP rgbdSensor_nws_yarp.
constexpr int protocolVersionMajor = 1;

spdlog::logger& logger()
{
    static const auto instance = [] {
        auto created = dinrail::createOrGetLogger("dinrail.rgbd_nwc_yarp");
        return created ? created : spdlog::default_logger();
    }();
    return *instance;
}

std::string stringParameter(const yarp::os::Searchable& config,
                            const std::string& key,
                            const std::string& fallback = "")
{
    return config.check(key) ? config.find(key).asString() : fallback;
}

} // namespace

DinrailRGBDSensorNWCYarp::RgbReader::RgbReader(DinrailRGBDSensorNWCYarp& owner)
    : m_owner(owner)
{
}

void DinrailRGBDSensorNWCYarp::RgbReader::onRead(yarp::sig::FlexImage& image)
{
    yarp::os::Stamp stamp;
    m_owner.m_rgbPort.getEnvelope(stamp);
    auto converted = std::make_shared<dinrail::Image>();
    if (!dinrail::toDinrailImage(image, stamp, *converted))
    {
        DINRAIL_WARN_THROTTLE(5.0,
                              &logger(),
                              "Received a color image with an unsupported pixel code.");
        return;
    }
    std::lock_guard<std::mutex> lock(m_owner.m_imagesMutex);
    m_owner.m_latestRgb = std::move(converted);
    m_owner.updatePair();
}

DinrailRGBDSensorNWCYarp::DepthReader::DepthReader(DinrailRGBDSensorNWCYarp& owner)
    : m_owner(owner)
{
}

void DinrailRGBDSensorNWCYarp::DepthReader::onRead(yarp::sig::ImageOf<yarp::sig::PixelFloat>& image)
{
    const auto arrival = Clock::now();
    yarp::os::Stamp stamp;
    m_owner.m_depthPort.getEnvelope(stamp);
    auto converted = std::make_shared<dinrail::DepthImage>();
    dinrail::toDinrailImage(image, stamp, *converted);
    std::lock_guard<std::mutex> lock(m_owner.m_imagesMutex);
    m_owner.m_latestDepth = std::move(converted);
    m_owner.m_lastDepthArrival = arrival;
    m_owner.updatePair();
}

void DinrailRGBDSensorNWCYarp::updatePair()
{
    if (m_latestRgb && m_latestDepth
        && m_latestRgb->stamp.sequenceNumber == m_latestDepth->stamp.sequenceNumber)
    {
        m_pairedRgb = m_latestRgb;
        m_pairedDepth = m_latestDepth;
    }
}

DinrailRGBDSensorNWCYarp::DinrailRGBDSensorNWCYarp()
    : m_rgbReader(*this)
    , m_depthReader(*this)
{
}

DinrailRGBDSensorNWCYarp::~DinrailRGBDSensorNWCYarp()
{
    close();
}

bool DinrailRGBDSensorNWCYarp::open(yarp::os::Searchable& config)
{
    if (config.check("frame_timeout"))
    {
        const double timeout = config.find("frame_timeout").asFloat64();
        if (timeout <= 0.0)
        {
            logger().error("The parameter 'frame_timeout' must be positive.");
            return false;
        }
        m_frameTimeout = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double>(timeout));
    }

    if (!connectServer(config) || !readParameters())
    {
        close();
        return false;
    }
    return true;
}

bool DinrailRGBDSensorNWCYarp::connectServer(const yarp::os::Searchable& config)
{
    const std::string remoteDepthPort = stringParameter(config, "remote_depth_port");
    const std::string localDepthPort = stringParameter(config, "local_depth_port");
    m_remoteRpcPort = stringParameter(config, "remote_rpc_port");
    const std::string localRpcPort = stringParameter(config, "local_rpc_port");
    if (remoteDepthPort.empty() || localDepthPort.empty() || m_remoteRpcPort.empty()
        || localRpcPort.empty())
    {
        logger().error("The parameters 'remote_depth_port', 'local_depth_port', "
                       "'remote_rpc_port' and 'local_rpc_port' are mandatory.");
        return false;
    }
    const std::string remoteImagePort = stringParameter(config, "remote_image_port");
    const std::string localImagePort = stringParameter(config, "local_image_port");
    m_hasRgb = !remoteImagePort.empty();
    if (m_hasRgb && localImagePort.empty())
    {
        logger().error("The parameter 'local_image_port' is mandatory when 'remote_image_port' "
                       "is set.");
        return false;
    }

    m_depthPort.useCallback(m_depthReader);
    if (!m_depthPort.open(localDepthPort))
    {
        logger().error("Unable to open the port {}.", localDepthPort);
        return false;
    }
    const std::string depthCarrier = stringParameter(config, "depth_carrier", "tcp");
    if (!yarp::os::Network::connect(remoteDepthPort, localDepthPort, depthCarrier))
    {
        logger().error("Unable to connect {} to {} with carrier {}.",
                       remoteDepthPort,
                       localDepthPort,
                       depthCarrier);
        return false;
    }

    if (m_hasRgb)
    {
        m_rgbPort.useCallback(m_rgbReader);
        if (!m_rgbPort.open(localImagePort))
        {
            logger().error("Unable to open the port {}.", localImagePort);
            return false;
        }
        const std::string imageCarrier = stringParameter(config, "image_carrier", "tcp");
        if (!yarp::os::Network::connect(remoteImagePort, localImagePort, imageCarrier))
        {
            logger().error("Unable to connect {} to {} with carrier {}.",
                           remoteImagePort,
                           localImagePort,
                           imageCarrier);
            return false;
        }
    }

    if (!m_rpcPort.open(localRpcPort) || !m_rpcPort.addOutput(m_remoteRpcPort))
    {
        logger().error("Unable to connect the RPC port {} to {}.", localRpcPort, m_remoteRpcPort);
        return false;
    }

    yarp::os::Bottle command;
    yarp::os::Bottle response;
    command.addVocab32(VOCAB_RGBD_SENSOR);
    command.addVocab32(VOCAB_GET);
    command.addVocab32(VOCAB_RGBD_PROTOCOL_VERSION);
    if (!m_rpcPort.write(command, response) || response.get(3).asInt32() != protocolVersionMajor)
    {
        logger().error("The server {} does not use version {} of the RGBD protocol.",
                       m_remoteRpcPort,
                       protocolVersionMajor);
        return false;
    }
    return true;
}

bool DinrailRGBDSensorNWCYarp::readParameters()
{
    dinrail::yarp_proto::framegrabber::DepthVisualParams_Forwarder depth(m_rpcPort);
    yarp::os::Property intrinsics;
    if (depth.getDepthIntrinsicParam(intrinsics))
    {
        m_depthIntrinsics = dinrail::toDinrailIntrinsics(intrinsics,
                                                         depth.getDepthWidth(),
                                                         depth.getDepthHeight());
    }
    double nearPlane = 0.0;
    double farPlane = 0.0;
    if (depth.getDepthClipPlanes(nearPlane, farPlane))
    {
        m_clipPlanes = std::make_pair(nearPlane, farPlane);
    }

    dinrail::yarp_proto::framegrabber::RgbVisualParams_Forwarder rgb(m_rpcPort);
    intrinsics.clear();
    if (rgb.getRgbIntrinsicParam(intrinsics))
    {
        m_rgbIntrinsics
            = dinrail::toDinrailIntrinsics(intrinsics, rgb.getRgbWidth(), rgb.getRgbHeight());
    }

    yarp::os::Bottle command;
    yarp::os::Bottle response;
    command.addVocab32(VOCAB_RGBD_SENSOR);
    command.addVocab32(VOCAB_GET);
    command.addVocab32(VOCAB_EXTRINSIC_PARAM);
    yarp::sig::Matrix matrix(4, 4);
    dinrail::CameraExtrinsics extrinsics;
    if (m_rpcPort.write(command, response) && response.get(0).asVocab32() != VOCAB_FAILED
        && yarp::os::Property::copyPortable(response.get(3), matrix)
        && dinrail::toDinrailExtrinsics(matrix, extrinsics))
    {
        m_extrinsics = extrinsics;
    }

    if (!m_depthIntrinsics)
    {
        logger().warn("The server {} does not provide the depth intrinsics.", m_remoteRpcPort);
    }
    return true;
}

void DinrailRGBDSensorNWCYarp::closePorts()
{
    // Closing a port waits for its running callback.
    m_depthPort.close();
    m_rgbPort.close();
    m_rpcPort.close();
}

bool DinrailRGBDSensorNWCYarp::close()
{
    closePorts();
    std::lock_guard<std::mutex> lock(m_imagesMutex);
    m_latestRgb.reset();
    m_latestDepth.reset();
    m_pairedRgb.reset();
    m_pairedDepth.reset();
    return true;
}

dinrail::RGBDSensorStatus DinrailRGBDSensorNWCYarp::getSensorStatus()
{
    std::lock_guard<std::mutex> lock(m_imagesMutex);
    if (!m_latestDepth)
    {
        return dinrail::RGBDSensorStatus::NotReady;
    }
    return Clock::now() - m_lastDepthArrival > m_frameTimeout ? dinrail::RGBDSensorStatus::Timeout
                                                              : dinrail::RGBDSensorStatus::Ok;
}

std::string DinrailRGBDSensorNWCYarp::getLastErrorMessage()
{
    yarp::os::Bottle command;
    yarp::os::Bottle response;
    command.addVocab32(VOCAB_RGBD_SENSOR);
    command.addVocab32(VOCAB_GET);
    command.addVocab32(VOCAB_ERROR_MSG);
    if (!m_rpcPort.write(command, response))
    {
        return "Unable to communicate with the server " + m_remoteRpcPort;
    }
    return response.get(3).asString();
}

dinrail::Status DinrailRGBDSensorNWCYarp::getRgbIntrinsics(dinrail::CameraIntrinsics& intrinsics)
{
    if (!m_rgbIntrinsics)
    {
        return dinrail::Status(dinrail::StatusCode::Unavailable);
    }
    intrinsics = *m_rgbIntrinsics;
    return dinrail::Status();
}

dinrail::Status DinrailRGBDSensorNWCYarp::getDepthIntrinsics(dinrail::CameraIntrinsics& intrinsics)
{
    if (!m_depthIntrinsics)
    {
        return dinrail::Status(dinrail::StatusCode::Unavailable);
    }
    intrinsics = *m_depthIntrinsics;
    return dinrail::Status();
}

dinrail::Status
DinrailRGBDSensorNWCYarp::getDepthToRgbExtrinsics(dinrail::CameraExtrinsics& extrinsics)
{
    if (!m_extrinsics)
    {
        return dinrail::Status(dinrail::StatusCode::Unavailable);
    }
    extrinsics = *m_extrinsics;
    return dinrail::Status();
}

dinrail::Status DinrailRGBDSensorNWCYarp::getDepthClipPlanes(double& nearPlane, double& farPlane)
{
    if (!m_clipPlanes)
    {
        return dinrail::Status(dinrail::StatusCode::Unavailable);
    }
    nearPlane = m_clipPlanes->first;
    farPlane = m_clipPlanes->second;
    return dinrail::Status();
}

dinrail::Status DinrailRGBDSensorNWCYarp::getRgbImage(dinrail::Image& rgbImage)
{
    if (!m_hasRgb)
    {
        return dinrail::Status(dinrail::StatusCode::FailedPrecondition);
    }
    std::shared_ptr<const dinrail::Image> latest;
    {
        std::lock_guard<std::mutex> lock(m_imagesMutex);
        latest = m_latestRgb;
    }
    if (!latest)
    {
        return dinrail::Status(dinrail::StatusCode::Unavailable);
    }
    rgbImage = *latest;
    return dinrail::Status();
}

dinrail::Status DinrailRGBDSensorNWCYarp::getDepthImage(dinrail::DepthImage& depthImage)
{
    std::shared_ptr<const dinrail::DepthImage> latest;
    {
        std::lock_guard<std::mutex> lock(m_imagesMutex);
        latest = m_latestDepth;
    }
    if (!latest)
    {
        return dinrail::Status(dinrail::StatusCode::Unavailable);
    }
    depthImage = *latest;
    return dinrail::Status();
}

dinrail::Status
DinrailRGBDSensorNWCYarp::getImages(dinrail::Image& rgbImage, dinrail::DepthImage& depthImage)
{
    if (!m_hasRgb)
    {
        return dinrail::Status(dinrail::StatusCode::FailedPrecondition);
    }
    std::shared_ptr<const dinrail::Image> rgb;
    std::shared_ptr<const dinrail::DepthImage> depth;
    {
        std::lock_guard<std::mutex> lock(m_imagesMutex);
        rgb = m_pairedRgb;
        depth = m_pairedDepth;
    }
    if (!rgb || !depth)
    {
        return dinrail::Status(dinrail::StatusCode::Unavailable);
    }
    rgbImage = *rgb;
    depthImage = *depth;
    return dinrail::Status();
}

dinrail::Status DinrailRGBDSensorNWCYarp::getImagesStamp(dinrail::Stamp& stamp)
{
    std::lock_guard<std::mutex> lock(m_imagesMutex);
    const std::shared_ptr<const dinrail::DepthImage>& depth = m_hasRgb ? m_pairedDepth
                                                                       : m_latestDepth;
    if (!depth)
    {
        return dinrail::Status(dinrail::StatusCode::Unavailable);
    }
    stamp = depth->stamp;
    return dinrail::Status();
}
