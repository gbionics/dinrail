/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef YARP_DEV_CONTROLBOARDWRAPPER_CONTROLBOARDWRAPPERLOGCOMPONENT_H
#define YARP_DEV_CONTROLBOARDWRAPPER_CONTROLBOARDWRAPPERLOGCOMPONENT_H

#include <spdlog/spdlog.h>

#include <string_view>

spdlog::logger& controlBoardLogger();
bool controlBoardShouldLogThrottle(std::string_view key, double intervalSeconds);

#endif // YARP_DEV_CONTROLBOARDWRAPPER_CONTROLBOARDWRAPPERLOGCOMPONENT_H
