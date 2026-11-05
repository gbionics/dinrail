# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

function(add_dinrail_python_module)

  set(options )
  set(oneValueArgs NAME)
  set(multiValueArgs
    SOURCES
    HEADERS
    LINK_LIBRARIES
    TESTS)

  set(prefix "dinrail")

  cmake_parse_arguments(${prefix}
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN})

  set(name ${${prefix}_NAME})
  set(sources ${${prefix}_SOURCES})
  set(headers ${${prefix}_HEADERS})
  set(link_libraries ${${prefix}_LINK_LIBRARIES})
  set(tests ${${prefix}_TESTS})

  foreach(file ${headers})
    set_property(GLOBAL APPEND PROPERTY dinrail_pybind_headers ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  endforeach()

  foreach(file ${sources})
    set_property(GLOBAL APPEND PROPERTY dinrail_pybind_sources ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  endforeach()

  set_property(GLOBAL APPEND PROPERTY dinrail_pybind_include_dirs ${CMAKE_CURRENT_SOURCE_DIR}/include)

  set_property(GLOBAL APPEND PROPERTY dinrail_pybind_link_libraries ${link_libraries})

  foreach(test ${tests})
    set_property(GLOBAL APPEND PROPERTY DinrailBindings_TESTS
      ${CMAKE_CURRENT_SOURCE_DIR}/${test})
  endforeach()

  message(STATUS "Added files for bindings named ${name} in ${PROJECT_NAME}.")

endfunction()

add_subdirectory(core)

get_property(pybind_headers GLOBAL PROPERTY dinrail_pybind_headers)
get_property(pybind_sources GLOBAL PROPERTY dinrail_pybind_sources)
get_property(pybind_include_dirs GLOBAL PROPERTY dinrail_pybind_include_dirs)
get_property(pybind_link_libraries GLOBAL PROPERTY dinrail_pybind_link_libraries)

pybind11_add_module(pybind11_dinrail MODULE
  dinrail.cpp
  ${pybind_sources}
  ${pybind_headers}
  )

target_include_directories(pybind11_dinrail PUBLIC "$<BUILD_INTERFACE:${pybind_include_dirs}>")

target_link_libraries(pybind11_dinrail PRIVATE
  ${pybind_link_libraries})

# The generated Python dynamic module must have the same name as the pybind11
# module, i.e. `bindings`.
set_target_properties(pybind11_dinrail PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${DINRAIL_PYTHON_PACKAGE}"
  ARCHIVE_OUTPUT_DIRECTORY "${DINRAIL_PYTHON_PACKAGE}"
  RUNTIME_OUTPUT_DIRECTORY "${DINRAIL_PYTHON_PACKAGE}"
  PDB_OUTPUT_DIRECTORY "${DINRAIL_PYTHON_PACKAGE}"
  OUTPUT_NAME "bindings")

# Output package is:
#
# dinrail
# |-- __init__.py (generated from main bindings CMake file)
# `-- bindings.<cpython_extension>

install(TARGETS pybind11_dinrail DESTINATION ${PYTHON_INSTDIR})

# Add tests for the bindings
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
