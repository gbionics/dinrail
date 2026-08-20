// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "TestInteropCommon.h"

#include <sharedlibpp/SharedLibraryClassApi.h>

namespace dinrail::test
{

class TestInteropBeta final : public TestInteropPluginBase
{
public:
    TestInteropBeta()
        : TestInteropPluginBase("beta", {"beta_device"})
    {
    }
};

} // namespace dinrail::test

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_interop_testbeta,
                               dinrail::test::TestInteropBeta,
                               dinrail::IInteropPlugin)
