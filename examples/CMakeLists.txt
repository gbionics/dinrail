# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28.3)

project(dinrail-examples LANGUAGES C CXX)

include(GNUInstallDirs)

find_package(dinrail REQUIRED)

add_subdirectory(implement-your-first-device)
