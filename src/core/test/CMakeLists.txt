# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Integration tests for dinrail

find_package(Catch2 3 REQUIRED)
include(Catch)

# Test Property class against YARP::os::Property (only if YARP is enabled)
if(DINRAIL_USES_YARP)
    find_package(YARP REQUIRED COMPONENTS os)

    add_executable(PropertyComparisonTest PropertyComparisonTest.cpp)
    target_link_libraries(PropertyComparisonTest
        PRIVATE
            dinrail::dinrail
            YARP::YARP_os
            Catch2::Catch2WithMain
    )
    catch_discover_tests(PropertyComparisonTest)
endif()
