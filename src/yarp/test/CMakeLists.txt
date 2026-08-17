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

add_executable(ValueCompatibilityTest ValueCompatibilityTest.cpp)
target_link_libraries(ValueCompatibilityTest
    PRIVATE
        dinrail::dinrail
        YARP::YARP_os
        Catch2::Catch2WithMain
)
add_test(NAME ValueCompatibilityTest COMMAND ValueCompatibilityTest)

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
