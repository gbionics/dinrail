# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

project(dinrail-cli
        LANGUAGES C CXX
        DESCRIPTION "Built-in devices for dinrail.")

if(PROJECT_IS_TOP_LEVEL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../DinrailCMakeCommon.cmake)
    find_package(dinrail REQUIRED)
endif()

find_package(CLI11 REQUIRED)

add_executable(dinrail-cli
    dinrail.cpp
)

set_target_properties(dinrail-cli PROPERTIES
    OUTPUT_NAME dinrail
)

target_link_libraries(dinrail-cli
    PRIVATE
        dinrail::dinrail
        CLI11::CLI11
)

install(TARGETS dinrail-cli)
