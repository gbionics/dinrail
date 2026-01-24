# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# The library name must match the scheme from getSharedlibppLibraryNameFromDeviceName
# which returns "dinrail-device-" + deviceName
add_library(dinrail-device-fakeMotionControl SHARED
    FakeMotionControl.cpp
    FakeMotionControl.h
)

target_link_libraries(dinrail-device-fakeMotionControl
    PRIVATE
        dinrail::dinrail
        sharedlibpp::sharedlibpp
)

# Install the plugin
install(TARGETS dinrail-device-fakeMotionControl)
