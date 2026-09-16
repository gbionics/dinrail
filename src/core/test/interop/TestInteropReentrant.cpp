// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "TestInteropCommon.h"

#include <dinrail/Device.h>
#include <sharedlibpp/SharedLibraryClassApi.h>

#include <stdexcept>

namespace dinrail::test
{
namespace
{
void openNativeChild(Device& child)
{
    Parameters config;
    config.put("device", "testnative");
    config.put("dinrail_device_type", "dinrail");
    if (!child.open(config))
    {
        throw std::runtime_error("Could not open native child");
    }
}

class NativeAdapter final : public InterfaceAdapterBase<IAdaptedFooTest, IDevice>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    std::string adaptedTag() const override
    {
        return "native adapted";
    }
};
} // namespace

class TestInteropReentrant final : public TestInteropPluginBase
{
public:
    TestInteropReentrant()
        : TestInteropPluginBase("reentrant", {"reentrant_device"})
    {
    }

    std::unique_ptr<IDevice> createDevice(const Parameters& config) override
    {
        Device child;
        openNativeChild(child);

        // The first optional query must be safe while createDevice is active.
        ILateAdaptedFooTest* missing = nullptr;
        if (child.view(missing))
        {
            throw std::runtime_error("Unexpected optional interface");
        }
        IAdaptedFooTest* adapted = nullptr;
        if (!child.view(adapted) || adapted->adaptedTag() != "native adapted")
        {
            throw std::runtime_error("Nested query did not find this plugin's adapter");
        }
        return TestInteropPluginBase::createDevice(config);
    }

    void registerInterfaceAdapters(InterfaceAdapterRegistry& registry) override
    {
        if (m_registrationStarted)
        {
            throw std::logic_error("Registration must not be repeated or reentered");
        }
        m_registrationStarted = true;

        Device child;
        openNativeChild(child);
        ILateAdaptedFooTest* missing = nullptr;
        if (child.view(missing))
        {
            throw std::runtime_error("Unexpected optional interface during registration");
        }
        registry.add<IAdaptedFooTest, IDevice, NativeAdapter>();
    }

private:
    bool m_registrationStarted{false};
};
} // namespace dinrail::test

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_interop_testreentrant,
                               dinrail::test::TestInteropReentrant,
                               dinrail::IInteropPlugin)
