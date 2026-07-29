# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

add_executable(dinrail-runner)
add_executable(dinrail-runner-rt)

set(dinrail_runner_SRCS
  main.cpp
  Module.cpp
  Module.h
  RunnerLog.cpp
  RunnerLog.h
)

target_sources(dinrail-runner
  PRIVATE
    ${dinrail_runner_SRCS}
)
target_link_libraries(dinrail-runner
  PRIVATE
    dinrail::dinrail
    spdlog::spdlog
    YARP::YARP_os
    YARP::YARP_dev
    YARP::YARP_init
    YARP::YARP_robotinterface
)
target_compile_definitions(dinrail-runner
  PRIVATE
    DINRAIL_RUNNER_DEFAULT_USE_RT_SAFE_MEMORY_SETTINGS=false
)

target_sources(dinrail-runner-rt
  PRIVATE
    ${dinrail_runner_SRCS}
)
target_link_libraries(dinrail-runner-rt
  PRIVATE
    dinrail::dinrail
    spdlog::spdlog
    YARP::YARP_os
    YARP::YARP_dev
    YARP::YARP_init
    YARP::YARP_robotinterface
)
target_compile_definitions(dinrail-runner-rt
  PRIVATE
    DINRAIL_RUNNER_DEFAULT_USE_RT_SAFE_MEMORY_SETTINGS=true
)

install(TARGETS dinrail-runner dinrail-runner-rt)
