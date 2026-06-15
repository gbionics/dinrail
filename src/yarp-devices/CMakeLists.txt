# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

if(BUILD_TESTING)
	find_package(Catch2 REQUIRED)
endif()

add_subdirectory(test-helpers)

add_subdirectory(ControlBoardNWCDinRailYARPPorts)
add_subdirectory(ControlBoardNWSDinRailYARPPorts)
add_subdirectory(ControlBoardRemapperDinRail)
