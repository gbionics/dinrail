# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

add_executable(harness_device_controlBoard_nws_yarp
	controlBoard_nws_yarp_test.cpp
)

target_link_libraries(harness_device_controlBoard_nws_yarp
	PRIVATE
		dinrail::dinrail
		YARP::YARP_os
		YARP::YARP_sig
		YARP::YARP_dev
		Catch2::Catch2WithMain
		dinrail-yarp-dev-shared
)

target_include_directories(harness_device_controlBoard_nws_yarp
	PRIVATE
		${PROJECT_SOURCE_DIR}/src/yarp-devices
)
add_test(NAME harness_device_controlBoard_nws_yarp COMMAND harness_device_controlBoard_nws_yarp)

add_executable(harness_device_streamingMessagesParser
	streamingMessagesParser_test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../StreamingMessagesParser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/../ControlBoardLogComponent.cpp
)

target_link_libraries(harness_device_streamingMessagesParser
	PRIVATE
		dinrail::dinrail
		YARP::YARP_os
		YARP::YARP_sig
		YARP::YARP_dev
		Catch2::Catch2WithMain
		dinrail-yarp-dev-shared
		spdlog::spdlog
)

target_include_directories(harness_device_streamingMessagesParser
	PRIVATE
		${PROJECT_SOURCE_DIR}/src/yarp-devices
		${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_compile_definitions(harness_device_streamingMessagesParser
	PRIVATE
		LOG_COMPONENT="yarp.devices.controlBoard_nws_yarp"
)
add_test(NAME harness_device_streamingMessagesParser COMMAND harness_device_streamingMessagesParser)

