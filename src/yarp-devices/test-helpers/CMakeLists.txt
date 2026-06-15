# This helper library is just meant to provide some shared
# headers across the test of dinrail's YARP devices, and it is not
# meant to be installed
add_library(dinrail-yarp-dev-test-helpers INTERFACE)

add_library(dinrail::dinrail-yarp-dev-test-helpers ALIAS dinrail-yarp-dev-test-helpers)

# Include directories
target_include_directories(dinrail-yarp-dev-test-helpers
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
