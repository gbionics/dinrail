# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Fixtures emulating a third-party device plugin that provides an interface unknown to dinrail,
# together with the Python bindings of that interface.
set(_fixtures_output_dir "${CMAKE_CURRENT_BINARY_DIR}/plugin")

# The interface is shared by the device plugin and its bindings
add_library(dinrail-bindings-test-igreeter SHARED plugin/IGreeter.h plugin/IGreeter.cpp)
target_include_directories(dinrail-bindings-test-igreeter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/plugin)
set_target_properties(dinrail-bindings-test-igreeter PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

dinrail_add_device(dinrail-device-test_greeter plugin/TestGreeter.h plugin/TestGreeter.cpp)
target_link_libraries(dinrail-device-test_greeter PRIVATE dinrail-bindings-test-igreeter)

pybind11_add_module(dinrail_test_greeter MODULE plugin/GreeterBindings.cpp)
target_include_directories(dinrail_test_greeter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(dinrail_test_greeter PRIVATE dinrail::dinrail dinrail-bindings-test-igreeter)

set_target_properties(dinrail-bindings-test-igreeter dinrail-device-test_greeter dinrail_test_greeter
    PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${_fixtures_output_dir}"
        RUNTIME_OUTPUT_DIRECTORY "${_fixtures_output_dir}")

set_property(GLOBAL APPEND PROPERTY DinrailBindings_TESTS_ENVIRONMENT_MODIFICATION
    "DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-device-test_greeter>"
    "PYTHONPATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail_test_greeter>")

# On Windows, the dependencies of the device plugin are searched in PATH when it is loaded.
if(WIN32)
    set_property(GLOBAL APPEND PROPERTY DinrailBindings_TESTS_ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-bindings-test-igreeter>")
endif()

# Tests must find the built-in plugins from the build tree even when the project is not installed.
if(TARGET dinrail-device-dr_battery_fake)
    set_property(GLOBAL APPEND PROPERTY DinrailBindings_TESTS_ENVIRONMENT_MODIFICATION
        "DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-device-dr_battery_fake>")
endif()
