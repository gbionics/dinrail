// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPCOMPATIBILITYLAYER_H
#define DINRAIL_YARPCOMPATIBILITYLAYER_H

#include <dinrail/ICompatibilityLayer.h>

namespace dinrail
{

class YarpCompatibilityLayer : public ICompatibilityLayer
{
public:
    YarpCompatibilityLayer();
    ~YarpCompatibilityLayer() override;

    std::unique_ptr<dinrail::IDevice> createDevice(const Parameters& config) override;
    ICompatibilityLayer* allocateInstance() const override;
    void registerInterfaceAdapters() override;
};

} // namespace dinrail

#endif // DINRAIL_YARPCOMPATIBILITYLAYER_H
