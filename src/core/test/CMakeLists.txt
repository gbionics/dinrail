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

add_executable(RuntimeDynamicCastTest RuntimeDynamicCastTest.cpp)
target_link_libraries(RuntimeDynamicCastTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_test(NAME RuntimeDynamicCastTest COMMAND RuntimeDynamicCastTest)

# Test-only interop plugins used to exercise interop discovery and the interop
# device-creation fallback used by view().
add_library(dinrail-interop-testalpha MODULE interop/TestInteropAlpha.cpp)
add_library(dinrail-interop-testbeta MODULE interop/TestInteropBeta.cpp)

foreach(_interop_plugin testalpha testbeta)
    target_link_libraries(dinrail-interop-${_interop_plugin}
        PRIVATE
            dinrail::dinrail
            sharedlibpp::sharedlibpp
    )
    target_include_directories(dinrail-interop-${_interop_plugin}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/interop
    )
endforeach()

add_executable(InteropPluginTest InteropPluginTest.cpp)
target_link_libraries(InteropPluginTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_dependencies(InteropPluginTest
    dinrail-interop-testalpha
    dinrail-interop-testbeta
)
add_test(NAME InteropPluginTest COMMAND InteropPluginTest)
set_tests_properties(InteropPluginTest PROPERTIES
    ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-interop-testbeta>")
