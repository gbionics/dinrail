# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

get_property(bindings_tests GLOBAL PROPERTY DinrailBindings_TESTS)
get_property(bindings_tests_environment GLOBAL PROPERTY DinrailBindings_TESTS_ENVIRONMENT_MODIFICATION)

# The working directory contains the dinrail package of the build tree.
add_test(NAME PythonBindingsUnitTest
    COMMAND ${Python3_EXECUTABLE} -m pytest -p no:cacheprovider ${bindings_tests}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if(bindings_tests_environment)
    set_tests_properties(PythonBindingsUnitTest PROPERTIES
        ENVIRONMENT_MODIFICATION "${bindings_tests_environment}")
endif()
