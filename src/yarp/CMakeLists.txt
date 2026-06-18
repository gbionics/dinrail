# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/DinrailGlobalVersion.cmake)
project(dinrail-yarp
        VERSION ${dinrail_global_VERSION}
        LANGUAGES C CXX
        DESCRIPTION "YARP compatibility support for dinrail.")

if(PROJECT_IS_TOP_LEVEL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../DinrailCMakeCommon.cmake)
    find_package(dinrail REQUIRED)
endif()

find_package(YARP REQUIRED COMPONENTS os dev)

# Minimal YARP conversion support for dinrail::Parameters <-> yarp::os::Property.
add_library(dinrail-yarp-conversions
    dinrail/YarpPropertyConverter.cpp
    dinrail/YarpPropertyConverter.h
)
add_library(dinrail::dinrail-yarp-conversions ALIAS dinrail-yarp-conversions)


target_link_libraries(dinrail-yarp-conversions    PUBLIC
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

# Install the library
install(TARGETS dinrail-yarp-conversions
    EXPORT ${PROJECT_NAME})

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dinrail
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")

include(InstallBasicPackageFiles)
# Workaround for https://github.com/robotology/ycm-cmake-modules/pull/481
set(_name "dinrail-yarp")
install_basic_package_files(${PROJECT_NAME}
                            COMPATIBILITY AnyNewerVersion
                            EXPORT ${PROJECT_NAME}
                            VARS_PREFIX ${PROJECT_NAME}
                            NAMESPACE dinrail-yarp::
                            CONFIG_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/dinrail-yarp-config.cmake.in
                            LOWERCASE_FILENAMES
                            DEPENDENCIES "YARP COMPONENTS os dev")
