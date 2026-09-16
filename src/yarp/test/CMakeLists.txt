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
    ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-interop-yarp>:$<TARGET_FILE_DIR:dinrail-device-dr_battery_fake>")
