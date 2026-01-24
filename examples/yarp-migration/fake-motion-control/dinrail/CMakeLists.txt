# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

find_package(CLI11 REQUIRED)

# dinrail fake-motion-control example
add_executable(fake-motion-control-dinrail main.cpp)

target_link_libraries(fake-motion-control-dinrail
    PRIVATE
        dinrail::dinrail
        CLI11::CLI11
)

install(TARGETS fake-motion-control-dinrail
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
