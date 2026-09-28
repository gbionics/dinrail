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

class FirstAdapter final : public dinrail::InterfaceAdapterBase<RequestedInterface, SourceInterface>
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

TEST_CASE("Interface adapter registry uses typed adapters in registration order")
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

namespace
{
struct MissingSource
{
    virtual ~MissingSource() = default;
};
class UnavailableAdapter final
    : public dinrail::InterfaceAdapterBase<RequestedInterface, MissingSource>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
};
class ViewedDevice final : public dinrail::IDevice, public dinrail::IInterfaceView
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
    void* viewInterface(const std::type_info& type) override
    {
        return type == typeid(SourceInterface) ? &m_source : nullptr;
    }

private:
    SourceInterface m_source;
};
} // namespace

TEST_CASE("Interface adapter registry skips unavailable sources")
{
    dinrail::InterfaceAdapterRegistry registry;
    registry.add<RequestedInterface, MissingSource, UnavailableAdapter>();
    TestDevice device;
    REQUIRE(registry.create(device, typeid(RequestedInterface)) == nullptr);
    registry.add<RequestedInterface, SourceInterface, SecondAdapter>();
    auto adapter = registry.create(device, typeid(RequestedInterface));
    REQUIRE(dynamic_cast<SecondAdapter*>(adapter.get()) != nullptr);
    REQUIRE(registry.create(device, typeid(MissingSource)) == nullptr);
}

TEST_CASE("Interface adapter registry resolves sources through IInterfaceView")
{
    dinrail::InterfaceAdapterRegistry registry;
    registry.add<RequestedInterface, SourceInterface, FirstAdapter>();
    ViewedDevice device;
    auto adapter = registry.create(device, typeid(RequestedInterface));
    REQUIRE(adapter != nullptr);
    REQUIRE(dynamic_cast<FirstAdapter*>(adapter.get()) != nullptr);
}
