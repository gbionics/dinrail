# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

add_executable(harness_device_dr_device_yarp dr_device_yarp_test.cpp)
target_link_libraries(harness_device_dr_device_yarp
  PRIVATE
    dinrail::dinrail
    dinrail::dinrail-yarp-conversions
    YARP::YARP_os
    YARP::YARP_dev
    Catch2::Catch2WithMain
)
add_dependencies(harness_device_dr_device_yarp
  dr_device_yarp
  dinrail-device-dr_multiplenalogsensors_fake
  dinrail-device-dr_rgbd_fake
)
add_test(NAME harness_device_dr_device_yarp COMMAND harness_device_dr_device_yarp)

# The dinrail YARP devices are found in the build tree, the YARP ones
# (multipleanalogsensorsserver and multipleanalogsensorsclient) in the YARP installation.
get_filename_component(_yarp_prefix "${YARP_DIR}/../../.." ABSOLUTE)
set_tests_properties(harness_device_dr_device_yarp
  PROPERTIES
    ENVIRONMENT "YARP_DATA_DIRS=${CMAKE_BINARY_DIR}/share/yarp:${_yarp_prefix}/share/yarp;DINRAIL_PLUGIN_PATH=$<TARGET_FILE_DIR:dinrail-device-dr_rgbd_fake>"
)
