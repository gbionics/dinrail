// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_INTERFACEADAPTERREGISTRY_H
#define DINRAIL_INTERFACEADAPTERREGISTRY_H

#include <dinrail/InterfaceAdapter.h>

#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace dinrail
{

/**
 * @brief Stores and creates interface adapter factories.
 *
 * Factories are registered once for each target interface. When an adapter
 * is requested, factories registered for that target are tried in registration
 * order until one succeeds.
 */
class InterfaceAdapterRegistry
{
public:
    template <class TargetInterface, class SourceInterface, class Adapter> void add();

    std::unique_ptr<dinrail::IInterfaceAdapter>
    create(dinrail::IDevice& device, const std::type_info& interfaceType) const
    {
        const auto candidates = m_factories.find(interfaceType);
        if (candidates == m_factories.end())
        {
            return nullptr;
        }
        for (auto factory : candidates->second)
        {
            if (auto adapter = factory(device))
            {
                return adapter;
            }
        }
        return nullptr;
    }

private:
    using Factory = std::unique_ptr<dinrail::IInterfaceAdapter> (*)(dinrail::IDevice&);
    std::unordered_map<std::type_index, std::vector<Factory>> m_factories;
};

template <class TargetInterface, class SourceInterface, class Adapter>
void InterfaceAdapterRegistry::add()
{
    static_assert(
        std::is_base_of_v<InterfaceAdapterBase<TargetInterface, SourceInterface>, Adapter>);
    m_factories[typeid(TargetInterface)].push_back(
        [](IDevice& device) -> std::unique_ptr<IInterfaceAdapter> {
            auto* source = viewSourceInterface<SourceInterface>(device);
            return source == nullptr ? nullptr : std::make_unique<Adapter>(*source);
        });
}

} // namespace dinrail

#endif // DINRAIL_INTERFACEADAPTERREGISTRY_H
