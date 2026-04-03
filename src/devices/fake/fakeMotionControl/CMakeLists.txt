# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

dinrail_add_device(
    dinrail-device-fakeMotionControl
    FakeMotionControl.h
    FakeMotionControl.cpp
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
