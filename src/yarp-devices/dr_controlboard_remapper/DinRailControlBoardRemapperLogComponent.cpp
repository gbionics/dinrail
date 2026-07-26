/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "DinRailControlBoardRemapperLogComponent.h"

#include <dinrail/SpdlogHelpers.h>

namespace dinrail::yarp_devices::remapper
{
spdlog::logger& controlBoardRemapperLogger()
{
    static const auto logger = dinrail::createOrGetLogger("dinrail.dr_controlboard_remapper");
    return *logger;
}

spdlog::logger& remoteControlBoardRemapperLogger()
{
    static const auto logger = dinrail::createOrGetLogger("dinrail.dr_controlboard_remapper_nwc_"
                                                          "yarp");
    return *logger;
}

} // namespace dinrail::yarp_devices::remapper
