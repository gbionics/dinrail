# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# YARP compatibility layer plugin
add_library(dinrail-compat-yarp MODULE
    YarpCompatibilityLayer.cpp
    YarpCompatibilityLayer.h
    YarpDeviceWrapper.cpp
    YarpDeviceWrapper.h
    YarpPropertyConverter.cpp
    YarpPropertyConverter.h
    YarpPositionDirectAdapter.cpp
    YarpPositionDirectAdapter.h
    YarpEncodersAdapter.cpp
    YarpEncodersAdapter.h
    YarpControlModeAdapter.cpp
    YarpControlModeAdapter.h
)

target_link_libraries(dinrail-compat-yarp PRIVATE
    dinrail::dinrail
    YARP::YARP_dev
    YARP::YARP_os
    sharedlibpp::sharedlibpp
)

# Install the compatibility layer plugin
install(TARGETS dinrail-compat-yarp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Add tests
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
