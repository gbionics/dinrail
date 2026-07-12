// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/InterfaceAdapterRegistry.h>

namespace dinrail
{

void InterfaceAdapterRegistry::registerAdapter(const std::type_info& interfaceType,
                                               InterfaceAdapterFactory factory)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_registry[std::type_index(interfaceType)] = std::move(factory);
}

void* InterfaceAdapterRegistry::queryAdapter(IDevice* device, const std::type_info& interfaceType) const
{
    if (device == nullptr)
    {
        return nullptr;
    }

    InterfaceAdapterFactory factory;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_registry.find(std::type_index(interfaceType));
        if (it == m_registry.end())
        {
            return nullptr;
        }
        factory = it->second;
    }

    if (factory)
    {
        return factory(device);
    }

    return nullptr;
}

} // namespace dinrail