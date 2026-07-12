// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPNATIVEVIEWHELPERS_H
#define DINRAIL_YARPNATIVEVIEWHELPERS_H

#include <dinrail/Device.h>

#include <yarp/dev/DeviceDriver.h>

namespace dinrail
{

template <class Interface>
bool viewInterfaceInYARPDeviceOpenedViaDinrail(Device& device, Interface*& interfaceView)
{
    interfaceView = nullptr;

    yarp::dev::DeviceDriver* deviceDriver = nullptr;
    if (!device.view(deviceDriver) || deviceDriver == nullptr)
    {
        return false;
    }

    interfaceView = dynamic_cast<Interface*>(deviceDriver->getImplementation());
    return interfaceView != nullptr;
}

} // namespace dinrail

#endif // DINRAIL_YARPNATIVEVIEWHELPERS_H