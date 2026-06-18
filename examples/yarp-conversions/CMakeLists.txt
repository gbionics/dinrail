# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

project(dinrail-example-implement-yarp-conversions-minimal LANGUAGES C CXX)

find_package(dinrail REQUIRED COMPONENTS yarp)

add_executable(yarp-conversions main.cpp)

target_link_libraries(yarp-conversions
    PRIVATE
        dinrail::dinrail-yarp-conversions
)

install(TARGETS yarp-conversions)
