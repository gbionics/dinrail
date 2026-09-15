# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28.3)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/DinrailGlobalVersion.cmake)
project(dinrail-yarp VERSION ${dinrail_global_VERSION} LANGUAGES C CXX DESCRIPTION "YARP compatibility support for dinrail.")

if(PROJECT_IS_TOP_LEVEL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DinrailCMakeCommon.cmake)
    find_package(dinrail REQUIRED)
endif()

find_package(YARP REQUIRED COMPONENTS os dev)

add_library(dinrail-yarp-conversions
    dinrail/YarpPropertyConverter.cpp dinrail/YarpPropertyConverter.h
    dinrail/YarpStampConverter.cpp dinrail/YarpStampConverter.h
    dinrail/YarpStatusConverter.cpp dinrail/YarpStatusConverter.h
    dinrail/BatteryAdapters.cpp dinrail/BatteryAdapters.h 
    dinrail/ControlBoardAdapters.cpp dinrail/ControlBoardAdapters.h
    dinrail/JoypadAdapters.cpp dinrail/JoypadAdapters.h
    dinrail/CommonAdapters.cpp dinrail/CommonAdapters.h
    dinrail/MultipleAnalogSensorsAdapters.cpp dinrail/MultipleAnalogSensorsAdapters.h
    dinrail/YarpDeviceFromDinrail.h)
add_library(dinrail::dinrail-yarp-conversions ALIAS dinrail-yarp-conversions)

target_link_libraries(dinrail-yarp-conversions PUBLIC dinrail::dinrail YARP::YARP_dev YARP::YARP_os)
target_include_directories(dinrail-yarp-conversions PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

add_library(dinrail-interop-yarp SHARED
    dinrail/DinrailDeviceFromYarp.cpp dinrail/DinrailDeviceFromYarp.h
    dinrail/YarpInteropPlugin.cpp dinrail/YarpInteropPlugin.h)
target_link_libraries(dinrail-interop-yarp PRIVATE dinrail::dinrail dinrail::dinrail-yarp-conversions YARP::YARP_dev YARP::YARP_os sharedlibpp::sharedlibpp)
target_include_directories(dinrail-interop-yarp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

install(TARGETS dinrail-yarp-conversions EXPORT ${PROJECT_NAME})
install(TARGETS dinrail-interop-yarp)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/YarpPropertyConverter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/YarpStatusConverter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/YarpStampConverter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/BatteryAdapters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/ControlBoardAdapters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/JoypadAdapters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/CommonAdapters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/MultipleAnalogSensorsAdapters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/YarpDeviceFromDinrail.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dinrail)

include(InstallBasicPackageFiles)
set(_name "dinrail-yarp")
install_basic_package_files(${PROJECT_NAME}
    COMPATIBILITY AnyNewerVersion EXPORT ${PROJECT_NAME} VARS_PREFIX ${PROJECT_NAME}
    NAMESPACE dinrail-yarp::
    CONFIG_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/dinrail-yarp-config.cmake.in
    LOWERCASE_FILENAMES DEPENDENCIES "YARP COMPONENTS os dev")
