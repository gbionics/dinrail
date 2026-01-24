# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Add the main dinrail library
add_subdirectory(core)

# If YARP is enabled, build the YARP compatibility layer
if(DINRAIL_USES_YARP)
    add_subdirectory(yarp)
endif()

# Add device plugins
add_subdirectory(devices)
