// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_ADAPTERREGISTRY_H
#define DINRAIL_ADAPTERREGISTRY_H

#include <functional>
#include <memory>
#include <typeinfo>
#include <unordered_map>

namespace dinrail
{

class IDevice;

class AdapterRegistry
{
public:
    using AdapterFactory = std::function<void*(IDevice*)>;

    static void registerAdapter(const std::type_info& interfaceType, AdapterFactory factory);
    static void* queryAdapter(IDevice* device, const std::type_info& interfaceType);

private:
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

    using RegistryMap = std::unordered_map<std::reference_wrapper<const std::type_info>,
                                           AdapterFactory,
                                           TypeInfoHash,
                                           TypeInfoEqual>;

    static RegistryMap& getRegistry();
};

} // namespace dinrail

#endif // DINRAIL_ADAPTERREGISTRY_H
