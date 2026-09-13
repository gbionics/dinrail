// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/InterfaceAdapterRegistry.h>

#include <catch2/catch_test_macros.hpp>

namespace
{
struct RequestedInterface
{
    virtual ~RequestedInterface() = default;
};
struct SourceInterface
{
    virtual ~SourceInterface() = default;
};

class TestDevice final : public dinrail::IDevice, public SourceInterface
{
public:
    bool open(const dinrail::Parameters&) override
    {
        return true;
    }
    bool close() override
    {
        return true;
    }
};

class FirstAdapter final
    : public dinrail::InterfaceAdapterBase<RequestedInterface, SourceInterface>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
};

class SecondAdapter final
    : public dinrail::InterfaceAdapterBase<RequestedInterface, SourceInterface>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
};
} // namespace

TEST_CASE("YARP adapter registry uses typed adapters in registration order")
{
    dinrail::InterfaceAdapterRegistry registry;
    registry.add<RequestedInterface, SourceInterface, FirstAdapter>();
    registry.add<RequestedInterface, SourceInterface, SecondAdapter>();

    TestDevice device;
    auto adapter = registry.create(device, typeid(RequestedInterface));
    REQUIRE(adapter != nullptr);
    REQUIRE(adapter->getInterface() != nullptr);
    REQUIRE(dynamic_cast<FirstAdapter*>(adapter.get()) != nullptr);
}
