# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause
if(NOT TARGET dinrail-device-dr_camera_fake)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../camera-devices/fake/dr_camera_fake
                     ${CMAKE_CURRENT_BINARY_DIR}/dr_camera_fake)
endif()
add_executable(DrCameraFakeYarpTest DrCameraFakeYarpTest.cpp)
target_link_libraries(DrCameraFakeYarpTest PRIVATE
    dinrail::dinrail-yarp-conversions YARP::YARP_init Catch2::Catch2WithMain)
add_dependencies(DrCameraFakeYarpTest dr_camera_fake dinrail-device-dr_camera_fake)
if(TARGET dinrail-interop-yarp)
    add_dependencies(DrCameraFakeYarpTest dinrail-interop-yarp)
endif()
add_test(NAME DrCameraFakeYarpTest COMMAND DrCameraFakeYarpTest)
set_tests_properties(DrCameraFakeYarpTest PROPERTIES
    ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-device-dr_camera_fake>;YARP_DATA_DIRS=${CMAKE_BINARY_DIR}/share/yarp")
