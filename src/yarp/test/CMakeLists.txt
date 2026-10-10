# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Catch2 3 REQUIRED)

add_executable(YarpPropertyConverterTest PropertyConverterTest.cpp)
target_link_libraries(YarpPropertyConverterTest
    PRIVATE
        dinrail::dinrail-yarp-conversions
        Catch2::Catch2WithMain
)
add_test(NAME YarpPropertyConverterTest COMMAND YarpPropertyConverterTest)

add_executable(YarpStampConverterTest YarpStampConverterTest.cpp)
target_link_libraries(YarpStampConverterTest
    PRIVATE
        dinrail::dinrail-yarp-conversions
        Catch2::Catch2WithMain
)
add_test(NAME YarpStampConverterTest COMMAND YarpStampConverterTest)

add_executable(YarpStatusConverterTest YarpStatusConverterTest.cpp)
target_link_libraries(YarpStatusConverterTest
    PRIVATE
        dinrail::dinrail-yarp-conversions
        YARP::YARP_dev
        Catch2::Catch2WithMain
)
add_test(NAME YarpStatusConverterTest COMMAND YarpStatusConverterTest)

add_executable(ValueCompatibilityTest ValueCompatibilityTest.cpp)
target_link_libraries(ValueCompatibilityTest
    PRIVATE
        dinrail::dinrail
        YARP::YARP_os
        Catch2::Catch2WithMain
)
add_test(NAME ValueCompatibilityTest COMMAND ValueCompatibilityTest)

add_executable(YarpDropInCompatibilityTest YarpDropInCompatibilityTest.cpp)
target_link_libraries(YarpDropInCompatibilityTest
    PRIVATE
        dinrail::dinrail
        dinrail::dinrail-yarp-conversions
        YARP::YARP_dev
        Catch2::Catch2WithMain
)
add_test(NAME YarpDropInCompatibilityTest COMMAND YarpDropInCompatibilityTest)
set_tests_properties(YarpDropInCompatibilityTest PROPERTIES
    ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-interop-yarp>;YARP_DATA_DIRS=${CMAKE_INSTALL_PREFIX}/share/yarp")

option(DINRAIL_USES_BipedalLocomotionFramework "Enable BLF deps for testing" OFF)

if (DINRAIL_USES_BipedalLocomotionFramework)
  find_package(BipedalLocomotionFramework REQUIRED)
  add_executable(BLFParametersHandlerCompatibilityTest BLFParametersHandlerCompatibilityTest.cpp)
  target_link_libraries(BLFParametersHandlerCompatibilityTest
      PRIVATE
          dinrail::dinrail
          dinrail::dinrail-yarp-conversions
          YARP::YARP_os
          YARP::YARP_robotinterface
          BipedalLocomotion::ParametersHandler
          BipedalLocomotion::ParametersHandlerYarpImplementation
          Catch2::Catch2WithMain
  )
  target_compile_definitions(BLFParametersHandlerCompatibilityTest PRIVATE
      DINRAIL_YARP_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
  )
  add_test(NAME BLFParametersHandlerCompatibilityTest COMMAND BLFParametersHandlerCompatibilityTest)
endif()

if(NOT TARGET dinrail-device-dr_battery_fake)
    # Standalone YARP builds also need the native battery fixture.
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../devices/fake/dr_battery_fake
                     ${CMAKE_CURRENT_BINARY_DIR}/dr_battery_fake)
endif()

add_executable(BatteryAdaptersTest BatteryAdaptersTest.cpp)
target_link_libraries(BatteryAdaptersTest PRIVATE
    dinrail-interop-yarp dinrail::dinrail-yarp-conversions Catch2::Catch2WithMain)
add_dependencies(BatteryAdaptersTest dinrail-device-dr_battery_fake)
add_test(NAME BatteryAdaptersTest COMMAND BatteryAdaptersTest)
set_tests_properties(BatteryAdaptersTest PROPERTIES
    ENVIRONMENT_MODIFICATION
        "DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-interop-yarp>;DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-device-dr_battery_fake>")

add_executable(YarpDeviceFromDinrailTest YarpDeviceFromDinrailTest.cpp)
target_include_directories(YarpDeviceFromDinrailTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../yarp-devices/dr_battery_fake)
target_link_libraries(YarpDeviceFromDinrailTest PRIVATE
    dinrail::dinrail-yarp-conversions Catch2::Catch2WithMain)
add_dependencies(YarpDeviceFromDinrailTest
    dinrail-device-dr_battery_fake)
add_test(NAME YarpDeviceFromDinrailTest COMMAND YarpDeviceFromDinrailTest)

# The wrapper plugin is only available when this directory is part of the
# monorepo build.  Standalone YARP builds still exercise the wrapper directly.
if(NOT PROJECT_IS_TOP_LEVEL)
    add_dependencies(YarpDeviceFromDinrailTest dr_battery_fake)
    target_compile_definitions(YarpDeviceFromDinrailTest PRIVATE
        DINRAIL_HAS_YARP_BATTERY_WRAPPER)
    set_tests_properties(YarpDeviceFromDinrailTest PROPERTIES
        ENVIRONMENT_MODIFICATION
            "DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-device-dr_battery_fake>;YARP_DATA_DIRS=path_list_prepend:${CMAKE_INSTALL_PREFIX}/share/yarp")
endif()

add_executable(YarpPreciselyTimedAdaptersTest YarpPreciselyTimedAdaptersTest.cpp)
target_link_libraries(YarpPreciselyTimedAdaptersTest PRIVATE
    dinrail-interop-yarp dinrail::dinrail-yarp-conversions Catch2::Catch2WithMain)
add_test(NAME YarpPreciselyTimedAdaptersTest COMMAND YarpPreciselyTimedAdaptersTest)

if(NOT TARGET dinrail-device-dr_controlboard_fake)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../devices/fake/dr_controlboard_fake
                     ${CMAKE_CURRENT_BINARY_DIR}/dr_controlboard_fake)
endif()

add_executable(YarpControlBoardAdaptersTest YarpControlBoardAdaptersTest.cpp)
target_include_directories(YarpControlBoardAdaptersTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../yarp-devices/dr_controlboard_fake)
target_link_libraries(YarpControlBoardAdaptersTest PRIVATE
    dinrail-interop-yarp dinrail::dinrail-yarp-conversions Catch2::Catch2WithMain)
add_dependencies(YarpControlBoardAdaptersTest dinrail-device-dr_controlboard_fake)
add_test(NAME YarpControlBoardAdaptersTest COMMAND YarpControlBoardAdaptersTest)
set_tests_properties(YarpControlBoardAdaptersTest PROPERTIES
    ENVIRONMENT_MODIFICATION
        "DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-interop-yarp>;DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-device-dr_controlboard_fake>")
if(NOT PROJECT_IS_TOP_LEVEL)
    add_dependencies(YarpControlBoardAdaptersTest dr_controlboard_fake)
    target_compile_definitions(YarpControlBoardAdaptersTest PRIVATE
        DINRAIL_HAS_YARP_CONTROLBOARD_WRAPPER)
    set_property(TEST YarpControlBoardAdaptersTest APPEND PROPERTY
        ENVIRONMENT_MODIFICATION "YARP_DATA_DIRS=path_list_prepend:${CMAKE_INSTALL_PREFIX}/share/yarp")
endif()

if(NOT TARGET dinrail-device-dr_multiplenalogsensors_fake)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../devices/fake/dr_multiplenalogsensors_fake
                     ${CMAKE_CURRENT_BINARY_DIR}/dr_multiplenalogsensors_fake)
endif()

add_executable(YarpMultipleAnalogSensorsAdaptersTest YarpMultipleAnalogSensorsAdaptersTest.cpp)
target_include_directories(YarpMultipleAnalogSensorsAdaptersTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../yarp-devices/dr_multiplenalogsensors_fake)
target_link_libraries(YarpMultipleAnalogSensorsAdaptersTest PRIVATE
    dinrail-interop-yarp dinrail::dinrail-yarp-conversions Catch2::Catch2WithMain)
add_dependencies(YarpMultipleAnalogSensorsAdaptersTest dinrail-device-dr_multiplenalogsensors_fake)
add_test(NAME YarpMultipleAnalogSensorsAdaptersTest COMMAND YarpMultipleAnalogSensorsAdaptersTest)
set_tests_properties(YarpMultipleAnalogSensorsAdaptersTest PROPERTIES
    ENVIRONMENT_MODIFICATION
        "DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-interop-yarp>;DINRAIL_PLUGIN_PATH=path_list_prepend:$<TARGET_FILE_DIR:dinrail-device-dr_multiplenalogsensors_fake>")
if(NOT PROJECT_IS_TOP_LEVEL)
    add_dependencies(YarpMultipleAnalogSensorsAdaptersTest dr_multiplenalogsensors_fake)
    target_compile_definitions(YarpMultipleAnalogSensorsAdaptersTest PRIVATE
        DINRAIL_HAS_YARP_MAS_WRAPPER)
    set_property(TEST YarpMultipleAnalogSensorsAdaptersTest APPEND PROPERTY
        ENVIRONMENT_MODIFICATION "YARP_DATA_DIRS=path_list_prepend:${CMAKE_INSTALL_PREFIX}/share/yarp")
endif()

add_executable(YarpJoypadAdaptersTest YarpJoypadAdaptersTest.cpp)
target_link_libraries(YarpJoypadAdaptersTest PRIVATE
    dinrail-interop-yarp dinrail::dinrail-yarp-conversions Catch2::Catch2WithMain)
add_test(NAME YarpJoypadAdaptersTest COMMAND YarpJoypadAdaptersTest)
