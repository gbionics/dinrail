// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "TestInteropCommon.h"

#include <sharedlibpp/SharedLibraryClassApi.h>

#include <stdexcept>

namespace dinrail::test
{
class LateFooAdapter final : public InterfaceAdapterBase<ILateAdaptedFooTest, IFooTest>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;

    std::string adaptedTag() const override
    {
        return "late:" + source().tag();
    }
};

class TestInteropLate final : public TestInteropPluginBase
{
public:
    TestInteropLate()
        : TestInteropPluginBase("late", {})
    {
    }

    void registerInterfaceAdapters(InterfaceAdapterRegistry& registry) override
    {
        if (m_registered)
        {
            throw std::logic_error("The late plugin must register only once per context");
        }
        registry.add<ILateAdaptedFooTest, IFooTest, LateFooAdapter>();
        m_registered = true;
    }

private:
    bool m_registered{false};
};
} // namespace dinrail::test

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_interop_testlate,
                               dinrail::test::TestInteropLate,
                               dinrail::IInteropPlugin)
