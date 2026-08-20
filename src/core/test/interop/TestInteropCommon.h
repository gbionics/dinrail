// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_TEST_INTEROP_COMMON_H
#define DINRAIL_TEST_INTEROP_COMMON_H

#include "IFooTest.h"

#include <dinrail/IDevice.h>
#include <dinrail/IInterfaceView.h>
#include <dinrail/IInteropPlugin.h>
#include <dinrail/Parameters.h>

#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

namespace dinrail::test
{

class FooImpl final : public IFooTest
{
public:
    explicit FooImpl(std::string tag)
        : m_tag(std::move(tag))
    {
    }

    std::string tag() const override
    {
        return m_tag;
    }

private:
    std::string m_tag;
};

// Device that exposes IFooTest only through IInterfaceView (not as a direct
// base) to exercise the interface resolution path of view().
class TestFooDevice final : public IDevice, public IInterfaceView
{
public:
    explicit TestFooDevice(std::string tag)
        : m_foo(std::move(tag))
    {
    }

    bool open(const Parameters& /*config*/) override
    {
        return true;
    }

    bool close() override
    {
        return true;
    }

    void* viewInterface(const std::type_info& interfaceType) override
    {
        if (interfaceType == typeid(IFooTest))
        {
            return static_cast<IFooTest*>(&m_foo);
        }
        return nullptr;
    }

private:
    FooImpl m_foo;
};

// Base for the test interop plugins: parameterized by tag and the set of device
// names it accepts.
class TestInteropPluginBase : public IInteropPlugin
{
public:
    TestInteropPluginBase(std::string tag, std::set<std::string> handledDevices)
        : m_tag(std::move(tag))
        , m_handledDevices(std::move(handledDevices))
    {
    }

    std::unique_ptr<IDevice> createDevice(const Parameters& config) override
    {
        if (!config.check<std::string>("device"))
        {
            return nullptr;
        }

        const std::string device = config.find("device").as<std::string>();
        if (m_handledDevices.find(device) == m_handledDevices.end())
        {
            return nullptr;
        }

        auto dev = std::make_unique<TestFooDevice>(m_tag);
        if (!dev->open(config))
        {
            return nullptr;
        }
        return dev;
    }

    std::vector<DeviceInfo> listDevices() const override
    {
        std::vector<DeviceInfo> devices;
        for (const auto& name : m_handledDevices)
        {
            devices.push_back({name, "test:" + m_tag});
        }
        return devices;
    }

private:
    std::string m_tag;
    std::set<std::string> m_handledDevices;
};

} // namespace dinrail::test

#endif // DINRAIL_TEST_INTEROP_COMMON_H
