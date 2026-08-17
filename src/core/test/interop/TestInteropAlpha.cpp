// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "TestInteropCommon.h"

#include <sharedlibpp/SharedLibraryClassApi.h>

namespace dinrail::test
{

class TestInteropAlpha final : public TestInteropPluginBase, public IInterfaceTranslationProvider
{
public:
    TestInteropAlpha()
        : TestInteropPluginBase("alpha", {"alpha_device"})
    {
    }

    std::unique_ptr<IInterfaceTranslation>
    createInterfaceTranslation(IDevice& device, const std::type_info& interfaceType) override
    {
        if (interfaceType != typeid(ITranslatedFooTest))
        {
            return nullptr;
        }

        auto* interfaceView = dynamic_cast<IInterfaceView*>(&device);
        if (interfaceView == nullptr)
        {
            return nullptr;
        }

        auto* source = static_cast<IFooTest*>(interfaceView->viewInterface(typeid(IFooTest)));
        if (source == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<FooTranslation>(*source);
    }
};

} // namespace dinrail::test

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_interop_testalpha,
                               dinrail::test::TestInteropAlpha,
                               dinrail::IInteropPlugin)
