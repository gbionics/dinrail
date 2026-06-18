# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28.3)

project(dinrail-examples LANGUAGES C CXX)

option(DINRAIL_EXAMPLES_USES_YARP "Enable dinrail examples that use YARP" OFF)

add_subdirectory(implement-your-first-device)

if(DINRAIL_EXAMPLES_USES_YARP)
  add_subdirectory(yarp-conversions)
endif()
