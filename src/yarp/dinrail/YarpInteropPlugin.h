// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPINTEROPPLUGIN_H
#define DINRAIL_YARPINTEROPPLUGIN_H

#include <dinrail/IInteropPlugin.h>

#include <vector>

namespace dinrail
{

class YarpInteropPlugin final : public IInteropPlugin
{
public:
    YarpInteropPlugin() = default;
    ~YarpInteropPlugin() override = default;

    std::unique_ptr<dinrail::IDevice> createDevice(const Parameters& config) override;

    std::vector<DeviceInfo> listDevices() const override;
};

} // namespace dinrail

#endif // DINRAIL_YARPINTEROPPLUGIN_H
