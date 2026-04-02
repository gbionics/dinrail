# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Minimal YARP conversion support for dinrail::Parameters <-> yarp::os::Property.
add_library(dinrail-yarp-conversions
    dinrail/YarpPropertyConverter.cpp
    dinrail/YarpPropertyConverter.h
)
add_library(dinrail::dinrail-yarp-conversions ALIAS dinrail-yarp-conversions)


target_link_libraries(dinrail-yarp-conversions
    PUBLIC
        dinrail::dinrail
        YARP::YARP_os
)

target_include_directories(dinrail-yarp-conversions
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
