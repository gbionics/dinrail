/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef YARP_CONTROLBOARDREMAPPERLOGCOMPONENT_H
#define YARP_CONTROLBOARDREMAPPERLOGCOMPONENT_H

#include <spdlog/spdlog.h>

namespace dinrail::yarp_devices::remapper
{
spdlog::logger& controlBoardRemapperLogger();

spdlog::logger& remoteControlBoardRemapperLogger();

} // namespace dinrail::yarp_devices::remapper

#endif // YARP_CONTROLBOARDREMAPPERLOGCOMPONENT_H
