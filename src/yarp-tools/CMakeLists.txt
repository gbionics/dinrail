# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

project(dinrail-yarp-tools
        LANGUAGES C CXX
        DESCRIPTION "dinrail tools that depend on YARP.")

if(PROJECT_IS_TOP_LEVEL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../DinrailCMakeCommon.cmake)
    find_package(dinrail REQUIRED)
endif()

find_package(YARP REQUIRED COMPONENTS os dev robotinterface)
find_package(spdlog REQUIRED)

add_subdirectory(dinrail-runner)
