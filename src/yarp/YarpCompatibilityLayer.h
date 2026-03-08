// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPCOMPATIBILITYLAYER_H
#define DINRAIL_YARPCOMPATIBILITYLAYER_H

#include <dinrail/ICompatibilityLayer.h>
#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>

#include <memory>

namespace dinrail
{

/**
 * @brief Compatibility layer for loading YARP devices.
 *
 * This compatibility layer allows dinrail to load and use YARP devices
 * when a native dinrail device is not available.
 */
class YarpCompatibilityLayer : public ICompatibilityLayer
{
public:
    YarpCompatibilityLayer();
    ~YarpCompatibilityLayer() override;

    /**
     * @brief Create a device instance by loading a YARP device.
     * @param config Configuration for the device
     * @return A unique pointer to the created device, or nullptr on failure
     */
    std::unique_ptr<dinrail::IDevice> createDevice(const Parameters& config) override;

    /**
     * @brief Allocate a new instance of this compatibility layer.
     * @return A new instance
     */
    ICompatibilityLayer* allocateInstance() const override;

    /**
     * @brief Register YARP interface adapters.
     *
     * Registers adapter factories for all supported YARP interfaces:
     * - IPositionDirect
     * - IEncoders
     * - IControlMode
     */
    void registerInterfaceAdapters() override;
};

} // namespace dinrail

#endif // DINRAIL_YARPCOMPATIBILITYLAYER_H
