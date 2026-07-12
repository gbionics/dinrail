// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPCOMPATIBILITYLAYER_H
#define DINRAIL_YARPCOMPATIBILITYLAYER_H

#include <dinrail/ICompatibilityLayer.h>

namespace dinrail
{

class YarpCompatibilityLayer final : public ICompatibilityLayer
{
public:
    YarpCompatibilityLayer() = default;
    ~YarpCompatibilityLayer() override = default;

    std::unique_ptr<dinrail::IDevice> createDevice(const Parameters& config) override;
    void registerInterfaceAdapters(InterfaceAdapterRegistry& registry) override;
};

} // namespace dinrail

#endif // DINRAIL_YARPCOMPATIBILITYLAYER_H