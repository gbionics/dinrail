# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(OpenCV REQUIRED COMPONENTS core imgproc)

dinrail_add_device(dinrail-device-dr_camera_fake DrCameraFake.h DrCameraFake.cpp)
target_link_libraries(dinrail-device-dr_camera_fake PRIVATE dinrail::dinrail-camera opencv_imgproc)
install(TARGETS dinrail-device-dr_camera_fake)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
