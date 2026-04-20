# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28.3)

# Main project
project(dinrail
        VERSION 0.0.0
        LANGUAGES C CXX
        DESCRIPTION "A lightweight library for robot device abstractions")

# Defines the CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros.
# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# Control where libraries and executables are placed during the build.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# To build shared libraries in Windows, we set CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS to TRUE.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Under MSVC, we set CMAKE_DEBUG_POSTFIX to "d" to add a trailing "d" to library
# built in debug mode.
if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

# Build position independent code.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Disable C and C++ compiler extensions.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(YCM REQUIRED)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DinrailAddDevice.cmake)

### Options
# Shared/Dynamic or Static library?
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)

# Build test related commands?
option(BUILD_TESTING "Create tests using CMake" OFF)
if(BUILD_TESTING)
    enable_testing()
endif()

# Build examples?
option(DINRAIL_BUILD_EXAMPLES "Build examples" OFF)

# Optional YARP support
option(DINRAIL_USES_YARP "Enable drop-in support for YARP device drivers" OFF)

# Enable RPATH support for installed binaries and libraries
include(AddInstallRPATHSupport)
add_install_rpath_support(BIN_DIRS "${CMAKE_INSTALL_FULL_BINDIR}"
                          LIB_DIRS "${CMAKE_INSTALL_FULL_LIBDIR}"
                          INSTALL_NAME_DIR "${CMAKE_INSTALL_FULL_LIBDIR}"
                          USE_LINK_PATH)

# Encourage user to specify a build type (e.g. Release, Debug, etc.), otherwise set it to Release.
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()

### Dependencies

# Find sharedlibpp for plugin system
find_package(sharedlibpp REQUIRED)

# Find YARP if compatibility layer is enabled
if(DINRAIL_USES_YARP)
    find_package(YARP REQUIRED COMPONENTS os dev)
endif()

### Compile- and install-related commands.
add_subdirectory(src)

# Create and install CMake configuration files for your project
include(InstallBasicPackageFiles)
install_basic_package_files(${PROJECT_NAME}
                            VERSION ${${PROJECT_NAME}_VERSION}
                            COMPATIBILITY AnyNewerVersion
                            EXPORT ${PROJECT_NAME}
                            VARS_PREFIX ${PROJECT_NAME}
                            NO_CHECK_REQUIRED_COMPONENTS_MACRO
                            INCLUDE_CONTENT "include(\"\${CMAKE_CURRENT_LIST_DIR}/DinrailAddDevice.cmake\")\n"
                            DEPENDENCIES sharedlibpp)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/DinrailAddDevice.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dinrail)

# Add the uninstall target
include(AddUninstallTarget)

# Examples are provided as a standalone project in examples/ and are meant to
# consume an externally-installed dinrail package.
if(DINRAIL_BUILD_EXAMPLES)
    message(WARNING
        "DINRAIL_BUILD_EXAMPLES is deprecated: examples are now standalone. "
        "Use the examples/pixi.toml workspace to configure and build them.")
endif()
