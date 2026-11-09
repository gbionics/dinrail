# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

set(H_PREFIX include/dinrail/bindings/core)

set(DINRAIL_BINDINGS_CORE_SRC
    src/Device.cpp
    src/IAxisInfo.cpp
    src/IBattery.cpp
    src/IEncoders.cpp
    src/IImpedanceAllSetPointsControl.cpp
    src/IJointFault.cpp
    src/IJoypadControl.cpp
    src/IMotor.cpp
    src/IMotorEncoders.cpp
    src/IPreciselyTimed.cpp
    src/Module.cpp
    src/MultipleAnalogSensorsInterfaces.cpp
    src/Parameters.cpp
    src/RuntimeContext.cpp
    src/Status.cpp
)

set(DINRAIL_BINDINGS_CORE_HDR
    ${H_PREFIX}/Device.h
    ${H_PREFIX}/IAxisInfo.h
    ${H_PREFIX}/IBattery.h
    ${H_PREFIX}/IEncoders.h
    ${H_PREFIX}/IImpedanceAllSetPointsControl.h
    ${H_PREFIX}/IJointFault.h
    ${H_PREFIX}/IJoypadControl.h
    ${H_PREFIX}/IMotor.h
    ${H_PREFIX}/IMotorEncoders.h
    ${H_PREFIX}/InterfaceRegistry.h
    ${H_PREFIX}/IPreciselyTimed.h
    ${H_PREFIX}/MethodWrapper.h
    ${H_PREFIX}/Module.h
    ${H_PREFIX}/MultipleAnalogSensorsInterfaces.h
    ${H_PREFIX}/Parameters.h
    ${H_PREFIX}/RuntimeContext.h
    ${H_PREFIX}/Status.h
)

add_dinrail_python_module(
    NAME DinrailCoreBindings
    SOURCES ${DINRAIL_BINDINGS_CORE_SRC}
    HEADERS ${DINRAIL_BINDINGS_CORE_HDR}
    LINK_LIBRARIES dinrail::dinrail
    TESTS tests/test_parameters.py tests/test_device.py tests/test_interfaces.py)

# Header-only helpers used by Python bindings of interfaces defined outside of dinrail
install(FILES ${H_PREFIX}/InterfaceRegistry.h ${H_PREFIX}/MethodWrapper.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dinrail/bindings/core)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
