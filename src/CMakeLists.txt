# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Add the main dinrail core library.
add_subdirectory(core)

# Add device plugins.
add_subdirectory(devices)

# Optional YARP Property conversion support.
if(DINRAIL_USES_YARP)
    add_subdirectory(yarp)
endif()
