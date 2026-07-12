# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

add_executable(dinrail-runner)
add_executable(dinrail-runner-rt)

set(dinrail_runner_SRCS
  main.cpp
  Module.cpp
  Module.h
)

target_sources(dinrail-runner
  PRIVATE
    ${dinrail_runner_SRCS}
)
target_link_libraries(dinrail-runner
  PRIVATE
    YARP::YARP_os
    YARP::YARP_dev
    YARP::YARP_init
    YARP::YARP_robotinterface
)

target_sources(dinrail-runner-rt
  PRIVATE
    ${dinrail_runner_SRCS}
)
target_link_libraries(dinrail-runner-rt
  PRIVATE
    YARP::YARP_os
    YARP::YARP_dev
    YARP::YARP_init
    YARP::YARP_robotinterface
)

install(TARGETS dinrail-runner dinrail-runner-rt)
