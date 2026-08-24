// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/IDevice.h>

#include <sharedlibpp/SharedLibraryClassApi.h>

namespace dinrail::test
{

class TestNativeDevice final : public IDevice
{
public:
    bool open(const Parameters&) override
    {
        return true;
    }

    bool close() override
    {
        return true;
    }
};

} // namespace dinrail::test

SHLIBPP_DEFINE_SHARED_SUBCLASS(dinrail_device_testnative,
                               dinrail::test::TestNativeDevice,
                               dinrail::IDevice)
