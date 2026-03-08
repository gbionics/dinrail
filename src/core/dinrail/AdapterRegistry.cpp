// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "AdapterRegistry.h"

namespace dinrail
{

void AdapterRegistry::registerAdapter(const std::type_info& interfaceType, AdapterFactory factory)
{
    getRegistry()[std::cref(interfaceType)] = std::move(factory);
}

void* AdapterRegistry::queryAdapter(IDevice* device, const std::type_info& interfaceType)
{
    if (device == nullptr)
    {
        return nullptr;
    }

    auto& registry = getRegistry();
    auto it = registry.find(std::cref(interfaceType));

    if (it != registry.end())
    {
        return it->second(device);
    }

    return nullptr;
}

AdapterRegistry::RegistryMap& AdapterRegistry::getRegistry()
{
    static RegistryMap registry;
    return registry;
}

} // namespace dinrail
