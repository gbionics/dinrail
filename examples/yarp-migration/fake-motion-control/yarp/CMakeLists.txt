# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(YARP REQUIRED COMPONENTS os dev)

add_executable(fake-motion-control-axisinfo-yarp main.cpp)

target_link_libraries(fake-motion-control-axisinfo-yarp
    PRIVATE
        YARP::YARP_os
        YARP::YARP_dev
)

install(TARGETS fake-motion-control-axisinfo-yarp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
