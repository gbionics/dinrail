# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# dinrail fake-motion-control example
add_executable(fake-motion-control-dinrail main.cpp)

target_link_libraries(fake-motion-control-dinrail
    PRIVATE
        dinrail::dinrail
)

install(TARGETS fake-motion-control-dinrail
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
