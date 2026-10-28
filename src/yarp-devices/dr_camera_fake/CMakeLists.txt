# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

dinrail_add_yarp_device(dr_camera_fake DrCameraFakeYarp DrCameraFakeYarp.h)
if(BUILD_TESTING AND TARGET dr_camera_fake)
    add_subdirectory(test)
endif()
