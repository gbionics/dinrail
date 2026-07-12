// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_ICOMPATIBILITYLAYER_H
#define DINRAIL_ICOMPATIBILITYLAYER_H

#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>

#include <memory>

namespace dinrail
{

class InterfaceAdapterRegistry;

/**
 * @brief Runtime compatibility layer for adapting non-dinrail devices.
 */
class ICompatibilityLayer
{
public:
    virtual ~ICompatibilityLayer() = default;

    /**
     * @brief Create a dinrail-compatible device from the provided configuration.
     *
     * Implementations are expected to return a non-null device only if the wrapped
     * backend device has been created successfully.
     */
    virtual std::unique_ptr<dinrail::IDevice> createDevice(const Parameters& config) = 0;

    /**
     * @brief Register runtime adapters provided by this compatibility layer.
     *
     * This is called once per RuntimeContext, allowing the layer to register
     * adapters with the context-local registry.
     */
    virtual void registerInterfaceAdapters(InterfaceAdapterRegistry& registry) = 0;
};

} // namespace dinrail

#endif // DINRAIL_ICOMPATIBILITYLAYER_H