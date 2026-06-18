# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

project(dinrail-devices
        LANGUAGES C CXX
        DESCRIPTION "Built-in devices for dinrail.")

if(PROJECT_IS_TOP_LEVEL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DinrailCMakeCommon.cmake)
    find_package(dinrail REQUIRED)
endif()

add_subdirectory(fake/fakeMotionControl)
