# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Catch2 3 REQUIRED)

add_executable(DrRgbdFakeTest DrRgbdFakeTest.cpp)
target_link_libraries(DrRgbdFakeTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_dependencies(DrRgbdFakeTest dinrail-device-dr_rgbd_fake)
add_test(NAME DrRgbdFakeTest COMMAND DrRgbdFakeTest)

# Tests must find plugins from the build tree even when the project is not installed.
set_tests_properties(DrRgbdFakeTest
    PROPERTIES
        ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-device-dr_rgbd_fake>"
)
