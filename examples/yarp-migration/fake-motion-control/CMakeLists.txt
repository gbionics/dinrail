# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# fake-motion-control example
# Demonstrates usage of a fake motion control device

# dinrail version of the example
add_subdirectory(dinrail)

# YARP version of the example (for comparison, requires YARP)
if(DINRAIL_USES_YARP)
    add_subdirectory(yarp)
endif()