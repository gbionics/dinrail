# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# These are test-only dependencies
find_package(Catch2 3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(iDynTree REQUIRED)


add_executable(GenericVectorTest GenericVectorTest.cpp)
target_link_libraries(GenericVectorTest
    PRIVATE
        dinrail::dinrail
        Eigen3::Eigen
        iDynTree::idyntree-core
        Catch2::Catch2WithMain
)
add_test(NAME GenericVectorTest COMMAND GenericVectorTest)