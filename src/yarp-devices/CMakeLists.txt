# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

project(dinrail-yarp-devices
        LANGUAGES C CXX
        DESCRIPTION "dinrail YARP devices.")

if(PROJECT_IS_TOP_LEVEL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../DinrailCMakeCommon.cmake)
    find_package(dinrail REQUIRED)
endif()

find_package(YARP REQUIRED COMPONENTS os dev)

if(BUILD_TESTING)
	find_package(Catch2 REQUIRED)
endif()

add_subdirectory(test-helpers)

add_subdirectory(dr_controlboard_nwc_yarp)
add_subdirectory(dr_controlboard_nws_yarp)
add_subdirectory(dr_controlboard_remapper)
