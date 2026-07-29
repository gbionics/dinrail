/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "DinRailControlBoardNWCYarpLogComponent.h"

#include <dinrail/SpdlogHelpers.h>

spdlog::logger& remoteControlBoardLogger()
{
    static const auto instance = dinrail::createOrGetLogger("dinrail.dr_controlboard_nwc_yarp");
    return *instance;
}
