# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

yarp_prepare_plugin(dr_controlboard_nwc_yarp
  CATEGORY device
  TYPE DinRailControlBoardNWCYarp
  INCLUDE DinRailControlBoardNWCYarp.h
  EXTRA_CONFIG
    WRAPPER=dr_controlboard_nws_yarp
  DEFAULT ON
  GENERATE_PARSER
)

if(NOT SKIP_dr_controlboard_nwc_yarp)
  yarp_add_plugin(dr_controlboard_nwc_yarp)

  target_sources(dr_controlboard_nwc_yarp
    PRIVATE
      DinRailControlBoardNWCYarp.cpp
      DinRailControlBoardNWCYarp.h
      DinRailControlBoardNWCYarp_ParamsParser.cpp
      DinRailControlBoardNWCYarp_ParamsParser.h
      DinRailControlBoardNWCYarpLogComponent.cpp
      DinRailControlBoardNWCYarpLogComponent.h
      stateExtendedReader.cpp
      stateExtendedReader.h
  )

  target_link_libraries(dr_controlboard_nwc_yarp
    PRIVATE
      dinrail-yarp-dev-shared
      YARP::YARP_os
      YARP::YARP_sig
      YARP::YARP_dev
  )

  yarp_install(
    TARGETS dr_controlboard_nwc_yarp
    EXPORT YARP_${YARP_PLUGIN_MASTER}
    COMPONENT ${YARP_PLUGIN_MASTER}
    LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
    ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
    YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
  )

  set_property(TARGET dr_controlboard_nwc_yarp PROPERTY FOLDER "Plugins/Device")

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()

endif()
