# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Catch2 3 REQUIRED)

add_executable(FakeMotionControlInterfacesTest FakeMotionControlInterfacesTest.cpp)
target_link_libraries(FakeMotionControlInterfacesTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_test(NAME FakeMotionControlInterfacesTest COMMAND FakeMotionControlInterfacesTest)

# Tests must find plugins from the build tree even when the project is not installed.
set_tests_properties(FakeMotionControlInterfacesTest
    PROPERTIES
        ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-device-fakeMotionControl>"
)

