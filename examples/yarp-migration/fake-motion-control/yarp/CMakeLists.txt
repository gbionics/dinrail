# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# YARP fake-motion-control example (for comparison with dinrail version)
# This requires YARP to be available

find_package(YARP REQUIRED COMPONENTS os dev)

add_executable(fake-motion-control-yarp main.cpp)

target_link_libraries(fake-motion-control-yarp
    PRIVATE
        YARP::YARP_os
        YARP::YARP_dev
)

install(TARGETS fake-motion-control-yarp
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
