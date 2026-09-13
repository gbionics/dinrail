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

add_executable(InterfaceAdapterRegistryTest InterfaceAdapterRegistryTest.cpp)
target_include_directories(InterfaceAdapterRegistryTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(InterfaceAdapterRegistryTest PRIVATE dinrail::dinrail Catch2::Catch2WithMain)
add_test(NAME InterfaceAdapterRegistryTest COMMAND InterfaceAdapterRegistryTest)

add_executable(YarpDeviceAdapterTest YarpDeviceAdapterTest.cpp)
target_include_directories(YarpDeviceAdapterTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(YarpDeviceAdapterTest PRIVATE
    dinrail-interop-yarp dinrail::dinrail-yarp-conversions Catch2::Catch2WithMain)
add_dependencies(YarpDeviceAdapterTest
    dinrail-device-dr_battery_fake
    dinrail-device-dr_controlboard_fake
    dinrail-device-dr_multiplenalogsensors_fake)
foreach(device IN ITEMS dr_battery_fake dr_controlboard_fake dr_multiplenalogsensors_fake)
    target_include_directories(YarpDeviceAdapterTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../yarp-devices/${device})
endforeach()
add_test(NAME YarpDeviceAdapterTest COMMAND YarpDeviceAdapterTest)
set_tests_properties(YarpDeviceAdapterTest PROPERTIES
    ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-interop-yarp>:$<TARGET_FILE_DIR:dinrail-device-dr_battery_fake>:$<TARGET_FILE_DIR:dinrail-device-dr_controlboard_fake>:$<TARGET_FILE_DIR:dinrail-device-dr_multiplenalogsensors_fake>;YARP_DATA_DIRS=${CMAKE_INSTALL_PREFIX}/share/yarp")
