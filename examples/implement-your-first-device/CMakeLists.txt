# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

project(dinrail-example-implement-your-first-device LANGUAGES C CXX)

find_package(dinrail REQUIRED)

include(GNUInstallDirs)

# 1. The INTERFACE header library for the MyInterface.h header,
#   which is needed by both the device library and the example executable.

add_library(MyInterface INTERFACE)
add_library(MyInterface::MyInterface ALIAS MyInterface)
target_include_directories(MyInterface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/MyInterfaceLib)


# 2. The library dinrail-device-MyDevice, which implements the device plugin.
dinrail_add_device(
    dinrail-device-MyDevice
    MyDevice.h
    MyDevice.cpp
)

# Linking dinrail is automatically handled by dinrail_add_device, but we need to link MyInterface as well.
target_link_libraries(dinrail-device-MyDevice PRIVATE MyInterface::MyInterface)

# Also, we need to install the device library, so that it can be discovered and loaded at runtime from the install tree.
install(TARGETS dinrail-device-MyDevice
    LIBRARY DESTINATION ${DINRAIL_INSTALL_MODULEDIR}
    RUNTIME DESTINATION ${DINRAIL_INSTALL_MODULEDIR}
)

# 2. The example executable - only links dinrail::dinrail; the device library
#    is discovered and loaded at runtime from the install tree.
add_executable(implement-your-first-device main.cpp)
target_link_libraries(implement-your-first-device
    PRIVATE
        dinrail::dinrail
        MyInterface::MyInterface
)

install(TARGETS implement-your-first-device)
