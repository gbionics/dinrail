# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

add_executable(harness_device_DinRailControlBoardNWCRtShm
	DinRailControlBoardNWCRtShm_test.cpp
)

target_link_libraries(harness_device_DinRailControlBoardNWCRtShm
	PRIVATE
		dinrail::dinrail
		YARP::YARP_os
		YARP::YARP_sig
		YARP::YARP_dev
		Catch2::Catch2WithMain
		dinrail-yarp-dev-shared
)
target_link_options(harness_device_DinRailControlBoardNWCRtShm
	PRIVATE
		-Wl,--no-as-needed,-lCatch2,-lCatch2Main,--as-needed
)
target_include_directories(harness_device_DinRailControlBoardNWCRtShm
	PRIVATE
		${PROJECT_SOURCE_DIR}/src/yarp-devices
)
add_test(NAME harness_device_DinRailControlBoardNWCRtShm COMMAND harness_device_DinRailControlBoardNWCRtShm)
