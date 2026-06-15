# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

yarp_prepare_plugin(
  dr_controlboard_nws_yarp
  CATEGORY device
  TYPE ControlBoard_nws_yarp
  INCLUDE ControlBoard_nws_yarp.h
  EXTRA_CONFIG
    WRAPPER=dr_controlboard_nws_yarp
  DEFAULT ON
  GENERATE_PARSER
)

if(NOT SKIP_dr_controlboard_nws_yarp)
  yarp_add_plugin(dr_controlboard_nws_yarp)

  target_compile_definitions(dr_controlboard_nws_yarp
    PRIVATE
      LOG_COMPONENT="yarp.devices.controlBoard_nws_yarp"
  )

  target_sources(dr_controlboard_nws_yarp
    PRIVATE
      ControlBoard_nws_yarp.cpp
      ControlBoard_nws_yarp.h
      ControlBoard_nws_yarp_ParamsParser.cpp
      ControlBoard_nws_yarp_ParamsParser.h
      RPCMessagesParser.cpp
      RPCMessagesParser.h
      StreamingMessagesParser.cpp
      StreamingMessagesParser.h
      ControlBoardLogComponent.cpp
      ControlBoardLogComponent.h
  )

  target_link_libraries(dr_controlboard_nws_yarp
    PRIVATE
      YARP::YARP_os
      YARP::YARP_sig
      YARP::YARP_dev
  )

  yarp_install(
    TARGETS dr_controlboard_nws_yarp
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
