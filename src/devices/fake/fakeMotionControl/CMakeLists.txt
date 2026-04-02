# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

add_library(dinrail-device-fakeMotionControl SHARED
    FakeMotionControl.cpp
    FakeMotionControl.h
)

target_link_libraries(dinrail-device-fakeMotionControl
    PRIVATE
        dinrail::dinrail
        sharedlibpp::sharedlibpp
)

install(TARGETS dinrail-device-fakeMotionControl)
