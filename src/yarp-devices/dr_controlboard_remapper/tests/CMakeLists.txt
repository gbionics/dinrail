# SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
# SPDX-License-Identifier: BSD-3-Clause

add_executable(harness_device_controlBoardRemapper_t1
	controlBoardRemapper_t1_test.cpp
)
target_link_libraries(harness_device_controlBoardRemapper_t1
	PRIVATE
		YARP::YARP_os
		YARP::YARP_sig
		YARP::YARP_dev
		Catch2::Catch2WithMain
		dinrail-yarp-dev-shared
)
target_link_options(harness_device_controlBoardRemapper_t1
	PRIVATE
		-Wl,--no-as-needed,-lCatch2,-lCatch2Main,--as-needed
)
target_include_directories(harness_device_controlBoardRemapper_t1
	PRIVATE
		${PROJECT_SOURCE_DIR}/src/yarp-devices
)

add_executable(harness_device_controlBoardRemapper_t2
	controlBoardRemapper_t2_test.cpp
)
target_link_libraries(harness_device_controlBoardRemapper_t2
	PRIVATE
		YARP::YARP_os
		YARP::YARP_sig
		YARP::YARP_dev
		Catch2::Catch2WithMain
		dinrail-yarp-dev-shared
)
target_link_options(harness_device_controlBoardRemapper_t2
	PRIVATE
		-Wl,--no-as-needed,-lCatch2,-lCatch2Main,--as-needed
)
target_include_directories(harness_device_controlBoardRemapper_t2
	PRIVATE
		${PROJECT_SOURCE_DIR}/src/yarp-devices
)

add_executable(harness_device_controlBoardRemapper_t3
	controlBoardRemapper_t3_test.cpp
)
target_link_libraries(harness_device_controlBoardRemapper_t3
	PRIVATE
		YARP::YARP_os
		YARP::YARP_sig
		YARP::YARP_dev
		Catch2::Catch2WithMain
		dinrail-yarp-dev-shared
)
target_link_options(harness_device_controlBoardRemapper_t3
	PRIVATE
		-Wl,--no-as-needed,-lCatch2,-lCatch2Main,--as-needed
)
target_include_directories(harness_device_controlBoardRemapper_t3
	PRIVATE
		${PROJECT_SOURCE_DIR}/src/yarp-devices
)

add_test(NAME harness_device_controlBoardRemapper_t1 COMMAND harness_device_controlBoardRemapper_t1)
set_tests_properties(harness_device_controlBoardRemapper_t1 PROPERTIES
    ENVIRONMENT "YARP_DATA_DIRS=${CMAKE_INSTALL_PREFIX}/share/yarp"
)
add_test(NAME harness_device_controlBoardRemapper_t2 COMMAND harness_device_controlBoardRemapper_t2)
set_tests_properties(harness_device_controlBoardRemapper_t2 PROPERTIES
    ENVIRONMENT "YARP_DATA_DIRS=${CMAKE_INSTALL_PREFIX}/share/yarp"
)
add_test(NAME harness_device_controlBoardRemapper_t3 COMMAND harness_device_controlBoardRemapper_t3)
set_tests_properties(harness_device_controlBoardRemapper_t3 PROPERTIES
    ENVIRONMENT "YARP_DATA_DIRS=${CMAKE_INSTALL_PREFIX}/share/yarp"
)
