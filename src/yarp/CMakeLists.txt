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

add_library(dinrail-compat-yarp MODULE
    dinrail/YarpAxisInfoAdapter.cpp
    dinrail/YarpAxisInfoAdapter.h
    dinrail/YarpCompatibilityLayer.cpp
    dinrail/YarpCompatibilityLayer.h
    dinrail/YarpDeviceWrapper.cpp
    dinrail/YarpDeviceWrapper.h
)

target_link_libraries(dinrail-compat-yarp
    PRIVATE
        dinrail::dinrail
        dinrail::dinrail-yarp-conversions
        YARP::YARP_dev
        YARP::YARP_os
        sharedlibpp::sharedlibpp
)

target_include_directories(dinrail-compat-yarp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS dinrail-compat-yarp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
