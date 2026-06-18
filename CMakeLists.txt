# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28.3)

# Main project
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DinrailGlobalVersion.cmake)
project(dinrail
        VERSION ${dinrail_global_VERSION}
        LANGUAGES C CXX
        DESCRIPTION "A lightweight library for robot device abstractions")

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DinrailCMakeCommon.cmake)

### Global Options

# Optional YARP support
option(DINRAIL_USES_YARP "Enable drop-in support for YARP device drivers" OFF)

add_subdirectory(src)
