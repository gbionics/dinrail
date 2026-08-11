// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "TestInteropCommon.h"

#include <sharedlibpp/SharedLibraryClassApi.h>

namespace dinrail::test
{

class TestInteropAlpha final : public TestInteropPluginBase
{
public:
    TestInteropAlpha()
        : TestInteropPluginBase("alpha", {"alpha_device"})
    {
    }
};

} // namespace dinrail::test

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_interop_testalpha,
                               dinrail::test::TestInteropAlpha,
                               dinrail::IInteropPlugin)
