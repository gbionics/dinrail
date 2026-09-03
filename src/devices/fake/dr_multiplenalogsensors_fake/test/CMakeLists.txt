# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Catch2 3 REQUIRED)

add_executable(DrMultipleAnalogSensorsFakeTest DrMultipleAnalogSensorsFakeTest.cpp)
target_link_libraries(DrMultipleAnalogSensorsFakeTest
    PRIVATE
        dinrail::dinrail
        Catch2::Catch2WithMain
)
add_dependencies(DrMultipleAnalogSensorsFakeTest dinrail-device-dr_multiplenalogsensors_fake)
add_test(NAME DrMultipleAnalogSensorsFakeTest COMMAND DrMultipleAnalogSensorsFakeTest)

set_tests_properties(DrMultipleAnalogSensorsFakeTest
    PROPERTIES
        ENVIRONMENT "DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-device-dr_multiplenalogsensors_fake>"
)
