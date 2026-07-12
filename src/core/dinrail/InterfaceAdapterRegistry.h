// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_INTERFACEADAPTERREGISTRY_H
#define DINRAIL_INTERFACEADAPTERREGISTRY_H

#include <functional>
#include <mutex>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace dinrail
{

class IDevice;

/**
 * @brief Runtime registry mapping interface types to adapter factories.
 */
class InterfaceAdapterRegistry
{
public:
    using InterfaceAdapterFactory = std::function<void*(IDevice*)>;

    void registerAdapter(const std::type_info& interfaceType, InterfaceAdapterFactory factory);
    void* queryAdapter(IDevice* device, const std::type_info& interfaceType) const;

private:
    using RegistryMap = std::unordered_map<std::type_index, InterfaceAdapterFactory>;

    mutable std::mutex m_mutex;
    RegistryMap m_registry;
};

} // namespace dinrail

#endif // DINRAIL_INTERFACEADAPTERREGISTRY_H