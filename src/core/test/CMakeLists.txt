# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Integration tests for dinrail

find_package(Catch2 3 REQUIRED)
# Eigen3 and iDynTree is just required for the tests
find_package(Eigen3 REQUIRED)
find_package(iDynTree REQUIRED)

add_executable(GenericVectorTest GenericVectorTest.cpp)
target_link_libraries(GenericVectorTest
    PRIVATE
        dinrail::dinrail
        Eigen3::Eigen
        Catch2::Catch2WithMain
        iDynTree::idyntree-core
)
add_test(NAME GenericVectorTest COMMAND GenericVectorTest)

#add_executable(ParametersTest ParametersTest.cpp)
#target_link_libraries(ParametersTest
#    PRIVATE
#        dinrail::dinrail
#        Eigen3::Eigen
#        Catch2::Catch2WithMain
#)
#add_test(NAME ParametersTest COMMAND ParametersTest)