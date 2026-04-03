# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Catch2 3 REQUIRED)

add_executable(FakeMotionControlAxisInfoTest FakeMotionControlAxisInfoTest.cpp)
target_link_libraries(FakeMotionControlAxisInfoTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_test(NAME FakeMotionControlAxisInfoTest COMMAND FakeMotionControlAxisInfoTest)

# Tests must find plugins from the build tree even when the project is not installed.
set_tests_properties(FakeMotionControlAxisInfoTest
    PROPERTIES
        ENVIRONMENT "DINRAIL_PLUGIN_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
)
