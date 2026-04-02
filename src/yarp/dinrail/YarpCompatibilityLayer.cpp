// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpCompatibilityLayer.h>

#include <dinrail/AdapterRegistry.h>
#include <dinrail/IAxisInfo.h>
#include <dinrail/YarpDeviceWrapper.h>
#include <dinrail/YarpPropertyConverter.h>

#include <sharedlibpp/SharedLibraryClassApi.h>

#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Property.h>

namespace dinrail
{

namespace
{
void* yarpAxisInfoAdapterFactory(IDevice* device)
{
    auto* wrapper = dynamic_cast<YarpDeviceWrapper*>(device);
    return wrapper ? wrapper->getAdapter(typeid(IAxisInfo)) : nullptr;
}
} // namespace

YarpCompatibilityLayer::YarpCompatibilityLayer() = default;
YarpCompatibilityLayer::~YarpCompatibilityLayer() = default;

std::unique_ptr<dinrail::IDevice> YarpCompatibilityLayer::createDevice(const Parameters& config)
{
    yarp::os::Property yarpConfig = YarpPropertyConverter::toYarpProperty(config);

    auto yarpDriver = std::make_unique<yarp::dev::PolyDriver>();
    if (!yarpDriver->open(yarpConfig))
    {
        return nullptr;
    }

    return std::make_unique<YarpDeviceWrapper>(std::move(yarpDriver));
}

ICompatibilityLayer* YarpCompatibilityLayer::allocateInstance() const
{
    return new YarpCompatibilityLayer();
}

void YarpCompatibilityLayer::registerInterfaceAdapters()
{
    AdapterRegistry::registerAdapter(typeid(IAxisInfo), yarpAxisInfoAdapterFactory);
}

} // namespace dinrail

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_compat_yarp,
                               dinrail::YarpCompatibilityLayer,
                               dinrail::ICompatibilityLayer)
