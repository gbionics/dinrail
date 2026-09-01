# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

dinrail_add_device(
    dinrail-device-dr_battery_fake
    DrBatteryFake.h
    DrBatteryFake.cpp
)

install(TARGETS dinrail-device-dr_battery_fake)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
