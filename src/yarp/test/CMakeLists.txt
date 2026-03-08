# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Tests for YARP compatibility layer

find_package(Catch2 3 REQUIRED)
find_package(BipedalLocomotionFramework REQUIRED)

# YarpPropertyConverter tests
add_executable(YarpPropertyConverterTest PropertyConverterTest.cpp)
target_link_libraries(YarpPropertyConverterTest
    PRIVATE
        dinrail::dinrail
        YARP::YARP_os
        Catch2::Catch2WithMain
)

# Need to link against YarpPropertyConverter.cpp since it's part of the plugin
target_sources(YarpPropertyConverterTest PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../YarpPropertyConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../YarpPropertyConverter.h
)

target_include_directories(YarpPropertyConverterTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_test(NAME YarpPropertyConverterTest COMMAND YarpPropertyConverterTest)

add_executable(ValueCompatibilityTest ValueCompatibilityTest.cpp)
target_link_libraries(ValueCompatibilityTest
    PRIVATE
        dinrail::dinrail
        YARP::YARP_os
        Catch2::Catch2WithMain
)
add_test(NAME ValueCompatibilityTest COMMAND ValueCompatibilityTest)

add_executable(BLFParametersHandlerCompatibilityTest BLFParametersHandlerCompatibilityTest.cpp)
target_link_libraries(BLFParametersHandlerCompatibilityTest
    PRIVATE
        dinrail::dinrail
        YARP::YARP_os
        BipedalLocomotion::ParametersHandler
        BipedalLocomotion::ParametersHandlerYarpImplementation
        Catch2::Catch2WithMain
)
target_sources(BLFParametersHandlerCompatibilityTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../YarpPropertyConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../YarpPropertyConverter.h
)
target_include_directories(BLFParametersHandlerCompatibilityTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_compile_definitions(BLFParametersHandlerCompatibilityTest PRIVATE
    DINRAIL_YARP_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
add_test(NAME BLFParametersHandlerCompatibilityTest COMMAND BLFParametersHandlerCompatibilityTest)

# Vocab compatibility tests
add_executable(VocabCompatibilityTest VocabCompatibilityTest.cpp)
target_link_libraries(VocabCompatibilityTest
    PRIVATE
        dinrail::dinrail
        YARP::YARP_dev
        YARP::YARP_os
        Catch2::Catch2WithMain
)

add_test(NAME VocabCompatibilityTest COMMAND VocabCompatibilityTest)
