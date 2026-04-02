# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(dinrail)

if(DINRAIL_USES_YARP)
    add_subdirectory(yarp)
endif()
