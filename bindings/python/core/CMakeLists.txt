# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

set(H_PREFIX include/dinrail/bindings/core)

set(DINRAIL_BINDINGS_CORE_SRC
    src/Device.cpp
    src/IBattery.cpp
    src/Module.cpp
    src/Parameters.cpp
    src/RuntimeContext.cpp
    src/Status.cpp
)

set(DINRAIL_BINDINGS_CORE_HDR
    ${H_PREFIX}/Device.h
    ${H_PREFIX}/IBattery.h
    ${H_PREFIX}/InterfaceRegistry.h
    ${H_PREFIX}/Module.h
    ${H_PREFIX}/Parameters.h
    ${H_PREFIX}/RuntimeContext.h
    ${H_PREFIX}/Status.h
)

add_dinrail_python_module(
    NAME DinrailCoreBindings
    SOURCES ${DINRAIL_BINDINGS_CORE_SRC}
    HEADERS ${DINRAIL_BINDINGS_CORE_HDR}
    LINK_LIBRARIES dinrail::dinrail
    TESTS tests/test_parameters.py tests/test_device.py)

# Header-only helper used by Python bindings of interfaces defined outside of dinrail
install(FILES ${H_PREFIX}/InterfaceRegistry.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dinrail/bindings/core)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
