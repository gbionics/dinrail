# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

yarp_prepare_plugin(dr_controlboard_remapper
  CATEGORY device
  TYPE ControlBoardRemapper
  INCLUDE ControlBoardRemapper.h
  EXTRA_CONFIG
    WRAPPER=dr_controlboard_nws_yarp
  DEFAULT ON
)

yarp_prepare_plugin(dr_controlboard_remapper_nwc_yarp
  CATEGORY device
  TYPE DinRailControlBoardRemapperNWCYarp
  INCLUDE DinRailControlBoardRemapperNWCYarp.h
  EXTRA_CONFIG
    WRAPPER=dr_controlboard_nws_yarp 
  DEFAULT ON
)

if(NOT SKIP_dr_controlboard_remapper OR NOT SKIP_dr_controlboard_remapper_nwc_yarp)
  yarp_add_plugin(dr_controlboard_remapper)

  target_sources(dr_controlboard_remapper
    PRIVATE
      ControlBoardRemapper.cpp
      ControlBoardRemapper.h
      ControlBoardRemapperHelpers.cpp
      ControlBoardRemapperHelpers.h
      ControlBoardRemapperLogComponent.cpp
      ControlBoardRemapperLogComponent.h
      DinRailControlBoardRemapperNWCYarp.cpp
      DinRailControlBoardRemapperNWCYarp.h)

  target_link_libraries(dr_controlboard_remapper
    PRIVATE
      YARP::YARP_os
      YARP::YARP_sig
      YARP::YARP_dev
  )

  yarp_install(
    TARGETS dr_controlboard_remapper
    LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
    ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
    YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
  )

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()

endif()
