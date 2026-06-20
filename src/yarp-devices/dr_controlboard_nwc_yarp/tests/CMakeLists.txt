# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

add_executable(harness_device_DinRailControlBoardNWCYarp
	DinRailControlBoardNWCYarp_test.cpp
)

target_link_libraries(harness_device_DinRailControlBoardNWCYarp
	PRIVATE
		YARP::YARP_os
		YARP::YARP_sig
		YARP::YARP_dev
		Catch2::Catch2WithMain
		dinrail-yarp-dev-shared
)
target_link_options(harness_device_DinRailControlBoardNWCYarp
	PRIVATE
		-Wl,--no-as-needed,-lCatch2,-lCatch2Main,--as-needed
)
target_include_directories(harness_device_DinRailControlBoardNWCYarp
	PRIVATE
		${PROJECT_SOURCE_DIR}/src/yarp-devices
)
add_test(NAME harness_device_DinRailControlBoardNWCYarp COMMAND harness_device_DinRailControlBoardNWCYarp)
