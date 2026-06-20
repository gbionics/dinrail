# This helper library provides code shared across dinrail YARP devices.

add_library(dinrail-yarp-dev-shared INTERFACE)

target_include_directories(dinrail-yarp-dev-shared
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(dinrail-yarp-dev-shared
    INTERFACE
        YARP::YARP_os
        YARP::YARP_sig
        YARP::YARP_dev
)

# Shared IDL 

yarp_idl_to_dir(
    INPUT_FILES idl/ControlBoardYARPJointData.thrift
    OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/idl_generated_code
    SOURCES_VAR DINRAIL_YARP_DEV_SHARED_IDL_SOURCES
    HEADERS_VAR DINRAIL_YARP_DEV_SHARED_IDL_HEADERS
    INCLUDE_DIRS_VAR DINRAIL_YARP_DEV_SHARED_IDL_INCLUDE_DIRS
    PLACEMENT MERGED
)

add_library(dinrail-yarp-dev-idl STATIC)

target_sources(dinrail-yarp-dev-idl PRIVATE ${DINRAIL_YARP_DEV_SHARED_IDL_SOURCES})

target_include_directories(dinrail-yarp-dev-idl
    PUBLIC
        $<BUILD_INTERFACE:${DINRAIL_YARP_DEV_SHARED_IDL_INCLUDE_DIRS}>
)

target_link_libraries(dinrail-yarp-dev-idl
    PUBLIC
        YARP::YARP_os
        YARP::YARP_sig
        YARP::YARP_dev
)
