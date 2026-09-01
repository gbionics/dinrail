# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Catch2 3 REQUIRED)

add_executable(DrBatteryFakeTest DrBatteryFakeTest.cpp)
target_link_libraries(DrBatteryFakeTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_dependencies(DrBatteryFakeTest dinrail-device-dr_battery_fake)
add_test(NAME DrBatteryFakeTest COMMAND DrBatteryFakeTest)

# Tests must find plugins from the build tree even when the project is not installed.
set_tests_properties(DrBatteryFakeTest
    PROPERTIES
        ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-device-dr_battery_fake>"
)
