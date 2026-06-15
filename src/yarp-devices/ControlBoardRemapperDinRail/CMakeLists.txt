# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

yarp_prepare_plugin(ControlBoardRemapperDinRail
  CATEGORY device
  TYPE ControlBoardRemapper
  INCLUDE ControlBoardRemapper.h
  EXTRA_CONFIG
    WRAPPER=ControlBoardNWSDinRailYARPPorts
  DEFAULT ON
)

yarp_prepare_plugin(RemoteControlBoardRemapperDinRail
  CATEGORY device
  TYPE RemoteControlBoardRemapper
  INCLUDE RemoteControlBoardRemapper.h
  EXTRA_CONFIG
    WRAPPER=ControlBoardNWSDinRailYARPPorts 
  DEFAULT ON
)

if(NOT SKIP_ControlBoardRemapperDinRail OR NOT SKIP_RemoteControlBoardRemapperDinRail)
  yarp_add_plugin(ControlBoardRemapperDinRail)

  target_sources(ControlBoardRemapperDinRail
    PRIVATE
      ControlBoardRemapper.cpp
      ControlBoardRemapper.h
      ControlBoardRemapperHelpers.cpp
      ControlBoardRemapperHelpers.h
      ControlBoardRemapperLogComponent.cpp
      ControlBoardRemapperLogComponent.h
      RemoteControlBoardRemapper.cpp
      RemoteControlBoardRemapper.h)

  target_link_libraries(ControlBoardRemapperDinRail
    PRIVATE
      YARP::YARP_os
      YARP::YARP_sig
      YARP::YARP_dev
  )

  yarp_install(
    TARGETS ControlBoardRemapperDinRail
    LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
    ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
    YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
  )

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()

endif()
