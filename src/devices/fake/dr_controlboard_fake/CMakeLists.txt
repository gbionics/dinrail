# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

dinrail_add_device(
    dinrail-device-dr_controlboard_fake
    FakeMotionControl.h
    FakeMotionControl.cpp
)

install(TARGETS dinrail-device-dr_controlboard_fake)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
