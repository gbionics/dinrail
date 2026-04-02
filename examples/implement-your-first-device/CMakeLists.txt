# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(sharedlibpp REQUIRED)

# 1. The device plugin - compiled as a SHARED library so that dinrail::Device
#    can load it at runtime via sharedlibpp.
add_library(dinrail-device-myDevice SHARED
    MyDevice.cpp
    MyDevice.h
    MyInterface.h
)

target_link_libraries(dinrail-device-myDevice
    PRIVATE
        dinrail::dinrail
        sharedlibpp::sharedlibpp
)

install(TARGETS dinrail-device-myDevice)

# 2. The example executable - only links dinrail::dinrail; the device library
#    is discovered and loaded at runtime from the install tree.
add_executable(implement-your-first-device main.cpp)

target_link_libraries(implement-your-first-device
    PRIVATE
        dinrail::dinrail
)

install(TARGETS implement-your-first-device
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
