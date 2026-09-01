# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28.3)

# Support building just dinrail-core as standalone project
set(DINRAIL_CMAKE_MODULES_PATH ${CMAKE_CURRENT_LIST_DIR}/../../cmake)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(${DINRAIL_CMAKE_MODULES_PATH}/DinrailGlobalVersion.cmake)
    project(dinrail
        VERSION ${dinrail_global_VERSION}
        LANGUAGES C CXX
        DESCRIPTION "A lightweight library for robot device abstractions")
    include(${DINRAIL_CMAKE_MODULES_PATH}/DinrailCMakeCommon.cmake)
    option(DINRAIL_ENABLE_INTEROP_PLUGINS "Enable support for loading and using interop plugins." ON)
    mark_as_advanced(DINRAIL_ENABLE_INTEROP_PLUGINS)
endif()

# Create the library
add_library(dinrail)

configure_file(
    dinrail/Config.h.in
    include/dinrail/Config.h
    @ONLY
)

target_sources(dinrail
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/include/dinrail/Config.h
    dinrail/Device.cpp
    dinrail/Device.h
    dinrail/IAxisInfo.h
    dinrail/IEncoders.h
    dinrail/IJointFault.h
    dinrail/IMotor.h
    dinrail/IMotorEncoders.h
    dinrail/IJoypadControl.h
    dinrail/Interfaces.cpp
    dinrail/IInterfaceView.h
    dinrail/IInteropPlugin.h
    dinrail/IPreciselyTimed.h
    dinrail/Parameters.cpp
    dinrail/PluginUtils.cpp
    dinrail/PluginUtils.h
    dinrail/Placeholder.cpp
    dinrail/RTMemoryGuard.h
    dinrail/RTMemoryGuard.cpp
    dinrail/RuntimeContext.cpp
    dinrail/RuntimeContext.h
    dinrail/RuntimeDynamicCast.cpp
    dinrail/RuntimeDynamicCast.h
    # Technically this header use spdlog, but as it is an header-only
    # header, we do not link spdlog here, and it is up to donwstream
    dinrail/SpdlogHelpers.h
    dinrail/Status.cpp
    dinrail/Status.h
    dinrail/Value.cpp
    dinrail/Version.h
)

add_library(${PROJECT_NAME}::dinrail ALIAS dinrail)

# Include directories
target_include_directories(dinrail
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(dinrail
    PUBLIC
        sharedlibpp::sharedlibpp
)

# To use std::span
target_compile_features(dinrail PUBLIC cxx_std_20)

if(DINRAIL_ENABLE_INTEROP_PLUGINS)
    # dinrail::runtimeDynamicCast relies on the MSVC or Itanium C++ ABI internals;
    # this definition is what makes RuntimeDynamicCast.cpp compile that ABI-specific
    # code in, otherwise a portable no-op implementation is compiled instead.
    target_compile_definitions(dinrail PRIVATE DINRAIL_ENABLE_INTEROP_PLUGINS)
endif()

# Install the library
install(TARGETS dinrail
    EXPORT ${PROJECT_NAME})

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dinrail
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/dinrail/Config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dinrail)

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
