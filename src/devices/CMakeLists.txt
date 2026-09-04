# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28.3)

project(dinrail-devices
        LANGUAGES C CXX
        DESCRIPTION "Built-in devices for dinrail.")

if(PROJECT_IS_TOP_LEVEL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DinrailCMakeCommon.cmake)
    find_package(dinrail REQUIRED)
endif()

add_subdirectory(fake/dr_battery_fake)
add_subdirectory(fake/dr_controlboard_fake)
add_subdirectory(fake/dr_multiplenalogsensors_fake)
