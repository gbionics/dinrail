// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_ICOMPATIBILITYLAYER_H
#define DINRAIL_ICOMPATIBILITYLAYER_H

#include <dinrail/IDevice.h>
#include <dinrail/Parameters.h>

#include <memory>

namespace dinrail
{

class ICompatibilityLayer
{
public:
    virtual ~ICompatibilityLayer() = default;

    virtual std::unique_ptr<dinrail::IDevice> createDevice(const Parameters& config) = 0;
    virtual ICompatibilityLayer* allocateInstance() const = 0;
    virtual void registerInterfaceAdapters() = 0;
};

} // namespace dinrail

#endif // DINRAIL_ICOMPATIBILITYLAYER_H
