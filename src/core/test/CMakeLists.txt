# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# These are test-only dependencies
find_package(Catch2 3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(iDynTree REQUIRED)

add_executable(StatusTest StatusTest.cpp)
target_link_libraries(StatusTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_test(NAME StatusTest COMMAND StatusTest)


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

# Test-only native MODULE plugin. On macOS, CMake gives MODULE libraries the
# .so suffix, unlike SHARED libraries, which use .dylib.
add_library(dinrail-device-testnative MODULE plugin/TestNativeDevice.cpp)
target_link_libraries(dinrail-device-testnative
    PRIVATE
        dinrail::dinrail
        sharedlibpp::sharedlibpp
)

add_executable(PluginLoadingTest PluginLoadingTest.cpp)
target_link_libraries(PluginLoadingTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_dependencies(PluginLoadingTest dinrail-device-testnative)
add_test(NAME PluginLoadingTest COMMAND PluginLoadingTest)
set_tests_properties(PluginLoadingTest PROPERTIES
    ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-device-testnative>"
)

# runtimeDynamicCast only performs real casts when built with the MSVC/Itanium
# ABI-specific implementation (see DINRAIL_ENABLE_INTEROP_PLUGINS).
if(DINRAIL_ENABLE_INTEROP_PLUGINS)
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
    add_library(dinrail-interop-testlate MODULE interop/TestInteropLate.cpp)
    # Keep the late plugin out of the automatically discovered library directory.
    set_target_properties(dinrail-interop-testlate PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/late-plugin"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/late-plugin")
    
    find_package(Threads REQUIRED)
    foreach(_interop_plugin testalpha testbeta testlate)
        target_link_libraries(dinrail-interop-${_interop_plugin}
            PRIVATE
                dinrail::dinrail
                sharedlibpp::sharedlibpp
                Threads::Threads
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
            Threads::Threads
    )
    add_dependencies(InteropPluginTest
        dinrail-interop-testalpha
        dinrail-interop-testbeta
        dinrail-interop-testlate
    )
    target_compile_definitions(InteropPluginTest PRIVATE
        DINRAIL_TEST_LATE_PLUGIN="$<TARGET_FILE:dinrail-interop-testlate>")
    add_test(NAME InteropPluginTest COMMAND InteropPluginTest)
    set_tests_properties(InteropPluginTest PROPERTIES
        ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-interop-testbeta>")

endif()

add_executable(InterfaceAdapterRegistryTest InterfaceAdapterRegistryTest.cpp)
target_link_libraries(InterfaceAdapterRegistryTest PRIVATE dinrail::dinrail Catch2::Catch2WithMain)
add_test(NAME InterfaceAdapterRegistryTest COMMAND InterfaceAdapterRegistryTest)
