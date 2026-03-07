// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_ICOMPATIBILITYLAYER_H
#define DINRAIL_ICOMPATIBILITYLAYER_H

#include <dinrail/Parameters.h>
#include <dinrail/IDevice.h>

#include <memory>

namespace dinrail
{

/**
 * @brief Interface for compatibility layers that can create device drivers
 * from other frameworks (e.g., YARP).
 *
 * Compatibility layers allow dinrail to load and wrap devices from other
 * frameworks when a native dinrail device is not available.
 */
class ICompatibilityLayer
{
public:
    virtual ~ICompatibilityLayer() = default;

    /**
     * @brief Create a device instance from the compatibility layer.
     * @param config Configuration for the device
     * @return A unique pointer to the created device, or nullptr on failure
     */
    virtual std::unique_ptr<dinrail::IDevice> createDevice(const Parameters& config) = 0;

    /**
     * @brief Allocate a new instance of this compatibility layer.
     * This is used by the plugin system.
     * @return A new instance
     */
    virtual ICompatibilityLayer* allocateInstance() const = 0;

    /**
     * @brief Register interface adapters with the global adapter registry.
     * 
     * This method is called once when the compatibility layer is first loaded.
     * Implementations should register all their adapter factories here.
     */
    virtual void registerInterfaceAdapters() = 0;
};

} // namespace dinrail

#endif // DINRAIL_ICOMPATIBILITYLAYER_H
