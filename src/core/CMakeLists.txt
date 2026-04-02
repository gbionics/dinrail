# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

set(LIBRARY_NAME dinrail)

# Create the library
add_library(${LIBRARY_NAME})

target_sources(${LIBRARY_NAME}
    PRIVATE
        dinrail/AdapterRegistry.cpp
        dinrail/AdapterRegistry.h
        dinrail/Constants.h
        dinrail/Device.cpp
        dinrail/Device.h
        dinrail/IAxisInfo.h
        dinrail/ICompatibilityLayer.h
        dinrail/Parameters.cpp
        dinrail/Placeholder.cpp
        dinrail/Value.cpp
)

add_library(${PROJECT_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

# Include directories
target_include_directories(${LIBRARY_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        sharedlibpp::sharedlibpp
)

# To use std::span
target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_20)

# Install the library
install(TARGETS ${LIBRARY_NAME}
    EXPORT ${PROJECT_NAME})

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dinrail
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")

# Add tests
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
