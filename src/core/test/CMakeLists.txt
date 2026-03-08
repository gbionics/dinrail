# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Integration tests for dinrail

find_package(Catch2 3 REQUIRED)
find_package(Eigen3 REQUIRED)

add_executable(ParametersTest ParametersTest.cpp)
target_link_libraries(ParametersTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_test(NAME ParametersTest COMMAND ParametersTest)

add_executable(GenericVectorTest GenericVectorTest.cpp)
target_link_libraries(GenericVectorTest
    PRIVATE
        dinrail::dinrail
        Eigen3::Eigen
        Catch2::Catch2WithMain
)
add_test(NAME GenericVectorTest COMMAND GenericVectorTest)