# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

set(LIBRARY_NAME dinrail)

# Create the library
add_library(${LIBRARY_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/IDevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Parameters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Parameters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/Value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dinrail/GenericVector.h
)

add_library(${PROJECT_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})

# Include directories
target_include_directories(${LIBRARY_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Link dependencies
target_compile_features(${LIBRARY_NAME} PUBLIC cxx_std_20)

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
