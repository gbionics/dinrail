// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "YarpCompatibilityLayer.h"
#include "YarpDeviceWrapper.h"
#include "YarpPropertyConverter.h"

#include <dinrail/AdapterRegistry.h>
#include <dinrail/IPositionDirect.h>
#include <dinrail/IEncoders.h>
#include <dinrail/IControlMode.h>

#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Property.h>

#include <sharedlibpp/SharedLibraryClass.h>

namespace dinrail
{

// Adapter factory functions - these check if the device is a YarpDeviceWrapper
// and return the appropriate adapter, or nullptr otherwise
namespace
{
    void* yarpPositionDirectAdapterFactory(IDevice* device)
    {
        auto* wrapper = dynamic_cast<YarpDeviceWrapper*>(device);
        return wrapper ? wrapper->getAdapter(typeid(IPositionDirect)) : nullptr;
    }

    void* yarpEncodersAdapterFactory(IDevice* device)
    {
        auto* wrapper = dynamic_cast<YarpDeviceWrapper*>(device);
        return wrapper ? wrapper->getAdapter(typeid(IEncoders)) : nullptr;
    }

    void* yarpControlModeAdapterFactory(IDevice* device)
    {
        auto* wrapper = dynamic_cast<YarpDeviceWrapper*>(device);
        return wrapper ? wrapper->getAdapter(typeid(IControlMode)) : nullptr;
    }
}

YarpCompatibilityLayer::YarpCompatibilityLayer() = default;
YarpCompatibilityLayer::~YarpCompatibilityLayer() = default;

std::unique_ptr<dinrail::IDevice> YarpCompatibilityLayer::createDevice(const Parameters& config)
{
    // Convert dinrail::Parameters to yarp::os::Property
    yarp::os::Property yarpConfig = YarpPropertyConverter::toYarpProperty(config);
    
    // Create YARP PolyDriver
    auto yarpDriver = std::make_unique<yarp::dev::PolyDriver>();
    if (!yarpDriver->open(yarpConfig))
    {
        return nullptr;
    }

    // Wrap the YARP device in a dinrail IDevice wrapper
    return std::make_unique<YarpDeviceWrapper>(std::move(yarpDriver));
}

ICompatibilityLayer* YarpCompatibilityLayer::allocateInstance() const
{
    return new YarpCompatibilityLayer();
}

void YarpCompatibilityLayer::registerInterfaceAdapters()
{
    // Register adapter factories for all YARP interfaces
    AdapterRegistry::registerAdapter(typeid(IPositionDirect), yarpPositionDirectAdapterFactory);
    AdapterRegistry::registerAdapter(typeid(IEncoders), yarpEncodersAdapterFactory);
    AdapterRegistry::registerAdapter(typeid(IControlMode), yarpControlModeAdapterFactory);
}

} // namespace dinrail

// Register this compatibility layer as a plugin
#include <sharedlibpp/SharedLibraryClassApi.h>
SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_compat_yarp, 
                               dinrail::YarpCompatibilityLayer,
                               dinrail::ICompatibilityLayer)
