// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_ADAPTERREGISTRY_H
#define DINRAIL_ADAPTERREGISTRY_H

#include <typeinfo>
#include <functional>
#include <unordered_map>
#include <memory>

namespace dinrail
{

class IDevice;

/**
 * @brief Registry for interface adapters.
 * 
 * Compatibility layers can register adapter factories that convert
 * devices from one system (e.g., YARP) to dinrail interfaces.
 * This allows multiple compat plugins to independently provide adapters
 * for different interface sets without modifying core classes.
 */
class AdapterRegistry
{
public:
    /**
     * @brief Adapter factory function type.
     * 
     * Takes an IDevice pointer and returns an interface pointer if the
     * device supports the interface through adaptation, or nullptr otherwise.
     */
    using AdapterFactory = std::function<void*(IDevice*)>;

    /**
     * @brief Register an adapter factory for a specific interface type.
     * 
     * @param interfaceType The type_info of the target interface
     * @param factory Function that creates adapters for devices
     */
    static void registerAdapter(const std::type_info& interfaceType, AdapterFactory factory);

    /**
     * @brief Query for an adapter for a specific interface type.
     * 
     * @param device The device to adapt
     * @param interfaceType The type_info of the target interface
     * @return Pointer to the adapted interface, or nullptr if no adapter available
     */
    static void* queryAdapter(IDevice* device, const std::type_info& interfaceType);

private:
    // Type-erased wrapper to allow type_info as key
    struct TypeInfoHash
    {
        std::size_t operator()(const std::type_info& info) const
        {
            return info.hash_code();
        }
    };

    struct TypeInfoEqual
    {
        bool operator()(const std::type_info& lhs, const std::type_info& rhs) const
        {
            return lhs == rhs;
        }
    };

    using RegistryMap = std::unordered_map<
        std::reference_wrapper<const std::type_info>,
        AdapterFactory,
        TypeInfoHash,
        TypeInfoEqual
    >;

    static RegistryMap& getRegistry();
};

} // namespace dinrail

#endif // DINRAIL_ADAPTERREGISTRY_H
