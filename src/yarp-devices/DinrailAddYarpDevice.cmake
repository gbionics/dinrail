# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# The native device remains a separate runtime plugin. The YARP wrapper opens it
# through dinrail::Device, so it does not link the native implementation.
function(dinrail_add_yarp_device name wrapper_class wrapper_header)
    set(source_target dinrail-device-${name})
    get_filename_component(wrapper_header "${wrapper_header}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    yarp_prepare_plugin(${name}
        CATEGORY device TYPE ${wrapper_class} INCLUDE ${wrapper_header} DEFAULT ON)
    if(NOT SKIP_${name})
        yarp_add_plugin(${name})
        target_sources(${name} PRIVATE "${wrapper_header}")
        target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_link_libraries(${name} PRIVATE dinrail::dinrail-yarp-conversions)
        add_dependencies(${name} ${source_target})
        yarp_install(TARGETS ${name}
            LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
            ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
            YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR})
    endif()
endfunction()
