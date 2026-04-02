# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# These are test-only dependencies
find_package(Catch2 3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(iDynTree REQUIRED)


add_executable(VectorProxyTest VectorProxyTest.cpp)
target_link_libraries(VectorProxyTest
    PRIVATE
        dinrail::dinrail
        Eigen3::Eigen
        iDynTree::idyntree-core
        Catch2::Catch2WithMain
)
add_test(NAME VectorProxyTest COMMAND VectorProxyTest)

add_executable(ParametersTest ParametersTest.cpp)
target_link_libraries(ParametersTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_test(NAME ParametersTest COMMAND ParametersTest)

add_executable(FakeMotionControlAxisInfoTest FakeMotionControlAxisInfoTest.cpp)
target_link_libraries(FakeMotionControlAxisInfoTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_test(NAME FakeMotionControlAxisInfoTest COMMAND FakeMotionControlAxisInfoTest)
