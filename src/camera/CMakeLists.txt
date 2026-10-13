# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28.3)

project(dinrail-camera LANGUAGES C CXX DESCRIPTION "Camera interfaces using OpenCV, separately from core libdinrail.")

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/DinrailGlobalVersion.cmake)
set(dinrail-camera_VERSION ${dinrail_global_VERSION})
if(PROJECT_IS_TOP_LEVEL)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DinrailCMakeCommon.cmake)
    find_package(dinrail REQUIRED)
endif()
find_package(OpenCV REQUIRED COMPONENTS core)
add_library(dinrail-camera dinrail/Interfaces.cpp)
add_library(dinrail::dinrail-camera ALIAS dinrail-camera)
target_link_libraries(dinrail-camera PUBLIC dinrail::dinrail opencv_core)
target_include_directories(dinrail-camera PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
install(TARGETS dinrail-camera EXPORT dinrail-camera)
install(DIRECTORY dinrail DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")
include(InstallBasicPackageFiles)
set(_name "dinrail-camera")
install_basic_package_files(dinrail-camera
    VERSION ${dinrail-camera_VERSION}
    COMPATIBILITY AnyNewerVersion
    EXPORT dinrail-camera
    NAMESPACE dinrail::
    DEPENDENCIES dinrail "OpenCV COMPONENTS core")
