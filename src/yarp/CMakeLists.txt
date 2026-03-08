# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Minimal YARP conversion support for dinrail::Parameters <-> yarp::os::Property.
add_library(dinrail-yarp-converter
    YarpPropertyConverter.cpp
    YarpPropertyConverter.h
)

target_link_libraries(dinrail-yarp-converter
    PUBLIC
        dinrail::dinrail
        YARP::YARP_os
)

target_include_directories(dinrail-yarp-converter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
