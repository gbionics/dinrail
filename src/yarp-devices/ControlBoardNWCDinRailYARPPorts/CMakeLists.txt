# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

yarp_prepare_plugin(dr_controlboard_nwc_yarp
  CATEGORY device
  TYPE RemoteControlBoard
  INCLUDE RemoteControlBoard.h
  EXTRA_CONFIG
    WRAPPER=dr_controlboard_nws_yarp
  DEFAULT ON
  GENERATE_PARSER
)

if(NOT SKIP_dr_controlboard_nwc_yarp)
  yarp_add_plugin(dr_controlboard_nwc_yarp)

  target_sources(dr_controlboard_nwc_yarp
    PRIVATE
      RemoteControlBoard.cpp
      RemoteControlBoard.h
      RemoteControlBoard_ParamsParser.cpp
      RemoteControlBoard_ParamsParser.h
      RemoteControlBoardLogComponent.cpp
      RemoteControlBoardLogComponent.h
      stateExtendedReader.cpp
      stateExtendedReader.h
  )

  target_link_libraries(dr_controlboard_nwc_yarp
    PRIVATE
      YARP::YARP_os
      YARP::YARP_sig
      YARP::YARP_dev
  )
  list(APPEND YARP_${YARP_PLUGIN_MASTER}_PRIVATE_DEPS
    YARP_os
    YARP_sig
    YARP_dev
  )

  yarp_install(
    TARGETS dr_controlboard_nwc_yarp
    EXPORT YARP_${YARP_PLUGIN_MASTER}
    COMPONENT ${YARP_PLUGIN_MASTER}
    LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
    ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
    YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
  )

  set(YARP_${YARP_PLUGIN_MASTER}_PRIVATE_DEPS ${YARP_${YARP_PLUGIN_MASTER}_PRIVATE_DEPS} PARENT_SCOPE)

  set_property(TARGET dr_controlboard_nwc_yarp PROPERTY FOLDER "Plugins/Device")

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()

endif()
