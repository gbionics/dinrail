# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

set(LIBRARY_NAME dinrail)

# Create the library
add_library(${LIBRARY_NAME}
    # Base functionality
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/IDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/ICompatibilityLayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Property.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Property.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/AdapterRegistry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/AdapterRegistry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Vocab.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Vocab.cpp
    # Interfaces 
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/IControlMode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/IEncoders.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/IPositionDirect.h
)

add_library(${PROJECT_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

# Include directories
target_include_directories(${LIBRARY_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Link dependencies
target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        sharedlibpp::sharedlibpp
)

# Install the library
install(TARGETS ${LIBRARY_NAME}
    EXPORT ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dinrail
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h")

# Add tests
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
