# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

#.rst:
# dinrail_add_device
# ------------------
#
# Add a dinrail device plugin as a module library and auto-generate the
# ``sharedlibpp`` registration translation unit.
#
# ::
#
#   dinrail_add_device(<target> <source> [<source> ...])
#
# The function is intentionally similar to ``add_library`` positional style:
# the first argument is the target name, and all remaining arguments are
# treated as sources.
#
# Parameters
# ^^^^^^^^^^
#
# ``<target>``
#   CMake target name for the plugin shared library.
#   It must start with ``dinrail-device-``.
#
#   The device name is extracted from the target suffix.
#   For example, target ``dinrail-device-MyDevice`` maps to device name
#   ``MyDevice`` and factory symbol ``dinrail_device_MyDevice``.
#
# ``<source> [<source> ...]``
#   Source files to compile into the plugin target.
#   The first header among these sources is used to infer:
#
#   - registration include path,
#   - concrete class name (from header stem).
#
#   If no header is present, the first source stem is used and ``.h`` is
#   assumed as header file for registration include.
#
# Behavior
# ^^^^^^^^
#
# - Creates ``add_library(<target> MODULE ...)``.
# - Generates an internal ``*_dinrail_registration.cpp`` file containing
#   ``SHLIBPP_DEFINE_SHARED_SUBCLASS(...)``.
# - Adds the directory of the first passed header as private include directory
#   (falls back to ``${CMAKE_CURRENT_SOURCE_DIR}`` if no header is passed).
# - Links privately against ``dinrail::dinrail`` and ``sharedlibpp::sharedlibpp``.
#
function(dinrail_add_device target)
    if(NOT target)
        message(FATAL_ERROR "dinrail_add_device: target is required")
    endif()

    if(ARGC LESS 2)
        message(FATAL_ERROR "dinrail_add_device: at least one source is required")
    endif()

    set(_dinrail_sources ${ARGN})

    if(NOT target MATCHES "^dinrail-device-")
        message(FATAL_ERROR "dinrail_add_device: target must start with 'dinrail-device-'")
    endif()

    string(REGEX REPLACE "^dinrail-device-" "" _dinrail_device_name "${target}")
    string(REPLACE "-" "_" _dinrail_factory_suffix "${_dinrail_device_name}")
    set(_dinrail_factory_symbol "dinrail_device_${_dinrail_factory_suffix}")

    set(_dinrail_class_header "")
    foreach(_src IN LISTS _dinrail_sources)
        get_filename_component(_src_ext "${_src}" EXT)
        if(_src_ext STREQUAL ".h" OR _src_ext STREQUAL ".hh" OR _src_ext STREQUAL ".hpp")
            set(_dinrail_class_header "${_src}")
            break()
        endif()
    endforeach()

    if(NOT _dinrail_class_header)
        list(GET _dinrail_sources 0 _first_source)
        get_filename_component(_first_stem "${_first_source}" NAME_WE)
        set(_dinrail_class_header "${_first_stem}.h")
    endif()

    get_filename_component(_dinrail_class_header_name "${_dinrail_class_header}" NAME)
    get_filename_component(_dinrail_class_header_dir "${_dinrail_class_header}" DIRECTORY)
    if(_dinrail_class_header_dir)
        if(IS_ABSOLUTE "${_dinrail_class_header_dir}")
            set(_dinrail_include_dir "${_dinrail_class_header_dir}")
        else()
            set(_dinrail_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/${_dinrail_class_header_dir}")
        endif()
    else()
        set(_dinrail_include_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    get_filename_component(_dinrail_class_name "${_dinrail_class_header}" NAME_WE)

    set(_dinrail_registration_file
        "${CMAKE_CURRENT_BINARY_DIR}/${target}_dinrail_registration.cpp")

    file(WRITE "${_dinrail_registration_file}"
"#include <cstdint>\n"
"#include <sharedlibpp/SharedLibraryClassApi.h>\n"
"#include \"${_dinrail_class_header_name}\"\n\n"
"using namespace dinrail;\n\n"
"SHLIBPP_DEFINE_SHARED_SUBCLASS(${_dinrail_factory_symbol}, ${_dinrail_class_name}, dinrail::IDevice)\n")

    add_library(${target} MODULE
        ${_dinrail_sources}
        "${_dinrail_registration_file}"
    )

    target_include_directories(${target}
        PRIVATE
            ${_dinrail_include_dir}
    )

    target_link_libraries(${target}
        PRIVATE
            dinrail::dinrail
            sharedlibpp::sharedlibpp
    )
endfunction()
