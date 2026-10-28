# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause
find_package(Catch2 3 REQUIRED)
add_executable(DrCameraFakeTest DrCameraFakeTest.cpp ../DrCameraFake.cpp)
target_link_libraries(DrCameraFakeTest PRIVATE dinrail::dinrail-camera opencv_imgproc Catch2::Catch2WithMain)
add_dependencies(DrCameraFakeTest dinrail-device-dr_camera_fake)
add_test(NAME DrCameraFakeTest COMMAND DrCameraFakeTest)
set_tests_properties(DrCameraFakeTest PROPERTIES
    ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-device-dr_camera_fake>")
