// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_IWRAPPEDDINRAILDEVICE_H
#define DINRAIL_IWRAPPEDDINRAILDEVICE_H

#include <dinrail/Device.h>

namespace dinrail
{

/**
 * @brief Implemented by the YARP devices that open a dinrail device.
 *
 * The dinrail-aware network wrappers attached to such a YARP device use the native interfaces
 * of the wrapped dinrail device, instead of going through the YARP interface adapters.
 */
class IWrappedDinrailDevice
{
public:
    virtual ~IWrappedDinrailDevice();

    virtual dinrail::Device& wrappedDinrailDevice() = 0;
};

} // namespace dinrail

#endif // DINRAIL_IWRAPPEDDINRAILDEVICE_H
