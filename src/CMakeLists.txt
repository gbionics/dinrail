# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Add the main dinrail core library.
add_subdirectory(core)

# Add device plugins.
add_subdirectory(devices)

# Add command line tools.
add_subdirectory(tools)

if(DINRAIL_USES_YARP)
    # Optional YARP Property conversion support.
    add_subdirectory(yarp)

    # YARP devices (i.e. devices that can be opened via yarp::dev::PolyDriver and yarprobotinterface)
    add_subdirectory(yarp-devices)

    # Dinrail YARP tools (i.e. tools for interacting with dinrail that currently depend on YARP)
    add_subdirectory(yarp-tools)
endif()
