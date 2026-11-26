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

# YARP framegrabber protocol, copied from YARP as it is not installed: it lets the dinrail RGBD
# network wrappers talk with the standard YARP RGBD clients and servers.
add_library(dinrail-yarp-dev-framegrabber-protocol STATIC
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/CameraVocabs.h
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/DepthVisualParams_Forwarder.cpp
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/DepthVisualParams_Forwarder.h
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/DepthVisualParams_Responder.cpp
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/DepthVisualParams_Responder.h
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/FrameGrabberControls_Forwarder.cpp
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/FrameGrabberControls_Forwarder.h
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/FrameGrabberControls_Responder.cpp
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/FrameGrabberControls_Responder.h
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/RgbVisualParams_Forwarder.cpp
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/RgbVisualParams_Forwarder.h
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/RgbVisualParams_Responder.cpp
    framegrabber_protocol/dinrail/yarp_proto/framegrabber/RgbVisualParams_Responder.h
)
set_target_properties(dinrail-yarp-dev-framegrabber-protocol PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(dinrail-yarp-dev-framegrabber-protocol
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/framegrabber_protocol>
)
target_link_libraries(dinrail-yarp-dev-framegrabber-protocol
    PUBLIC
        YARP::YARP_os
        YARP::YARP_sig
        YARP::YARP_dev
)
