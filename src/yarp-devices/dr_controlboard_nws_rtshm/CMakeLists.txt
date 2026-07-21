# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Threads REQUIRED)

yarp_prepare_plugin(dr_controlboard_nws_rtshm
  CATEGORY device
  TYPE DinRailControlBoardNWSRtShm
  INCLUDE DinRailControlBoardNWSRtShm.h
  EXTRA_CONFIG
    WRAPPER=dr_controlboard_nws_rtshm
  DEFAULT ON
)

if(NOT SKIP_dr_controlboard_nws_rtshm)
  yarp_add_plugin(dr_controlboard_nws_rtshm)

  target_compile_definitions(dr_controlboard_nws_rtshm
    PRIVATE
      LOG_COMPONENT="yarp.devices.controlBoard_nws_rtshm"
  )

  target_sources(dr_controlboard_nws_rtshm
    PRIVATE
      DinRailControlBoardNWSRtShm.cpp
      DinRailControlBoardNWSRtShm.h
      DinRailControlBoardNWSRtShm_ParamsParser.h
  )

  target_link_libraries(dr_controlboard_nws_rtshm
    PRIVATE
      dinrail::dinrail
      dinrail-rtshm
      YARP::YARP_os
      YARP::YARP_sig
      YARP::YARP_dev
      Threads::Threads
  )

  yarp_install(
    TARGETS dr_controlboard_nws_rtshm
    EXPORT YARP_${YARP_PLUGIN_MASTER}
    COMPONENT ${YARP_PLUGIN_MASTER}
    LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
    ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
    YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
  )

  set_property(TARGET dr_controlboard_nws_rtshm PROPERTY FOLDER "Plugins/Device")

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()

endif()
