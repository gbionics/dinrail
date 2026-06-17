# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Support building just dinrail-core as standalone project
set(DINRAIL_CMAKE_MODULES_PATH ${CMAKE_CURRENT_LIST_DIR}/../../cmake)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${DINRAIL_CMAKE_MODULES_PATH}/DinrailGlobalVersion.cmake)
    project(dinrail
        VERSION ${dinrail_global_VERSION}
        LANGUAGES C CXX
        DESCRIPTION "A lightweight library for robot device abstractions")
    include(${DINRAIL_CMAKE_MODULES_PATH}/DinrailCMakeCommon.cmake)
endif()

# Create the library
add_library(dinrail)

target_sources(dinrail
    PRIVATE
    dinrail/Device.cpp
    dinrail/Device.h
    dinrail/IAxisInfo.h
    dinrail/Parameters.cpp
    dinrail/PluginUtils.cpp
    dinrail/PluginUtils.h
    dinrail/Placeholder.cpp
    dinrail/Value.cpp
)

add_library(${PROJECT_NAME}::dinrail ALIAS dinrail)

# Include directories
target_include_directories(dinrail
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(dinrail
    PUBLIC
        sharedlibpp::sharedlibpp
)

# To use std::span
target_compile_features(dinrail PUBLIC cxx_std_20)

# Install the library
install(TARGETS dinrail
    EXPORT ${PROJECT_NAME})

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dinrail
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")

# Add tests
if(BUILD_TESTING)
    add_subdirectory(test)
endif()

include(InstallBasicPackageFiles)
# Workaround for https://github.com/robotology/ycm-cmake-modules/pull/481
set(_name "dinrail")
install_basic_package_files(${PROJECT_NAME}
                            VERSION ${${PROJECT_NAME}_VERSION}
                            COMPATIBILITY AnyNewerVersion
                            EXPORT ${PROJECT_NAME}
                            VARS_PREFIX ${PROJECT_NAME}
                            CONFIG_TEMPLATE ${CMAKE_CURRENT_SOURCE_DIR}/dinrail-config.cmake.in
                            DEPENDENCIES sharedlibpp)

install(FILES ${DINRAIL_CMAKE_MODULES_PATH}/DinrailAddDevice.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dinrail)

