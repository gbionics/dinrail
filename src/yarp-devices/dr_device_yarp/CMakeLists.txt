# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

yarp_prepare_plugin(
  dr_device_yarp
  CATEGORY device
  TYPE DrDeviceYarp
  INCLUDE DrDeviceYarp.h
  DEFAULT ON
)

if(NOT SKIP_dr_device_yarp)
  yarp_add_plugin(dr_device_yarp)

  target_sources(dr_device_yarp
    PRIVATE
      DrDeviceYarp.cpp
      DrDeviceYarp.h
  )

  target_link_libraries(dr_device_yarp
    PRIVATE
      dinrail::dinrail
      dinrail::dinrail-yarp-conversions
      spdlog::spdlog
      YARP::YARP_os
      YARP::YARP_dev
  )

  yarp_install(
    TARGETS dr_device_yarp
    EXPORT YARP_${YARP_PLUGIN_MASTER}
    COMPONENT ${YARP_PLUGIN_MASTER}
    LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
    ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
    YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
  )

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()
endif()
