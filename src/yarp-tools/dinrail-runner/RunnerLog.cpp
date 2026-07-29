/*
 * SPDX-FileCopyrightText: Generative Bionics S.R.L.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "RunnerLog.h"

#include <dinrail/SpdlogHelpers.h>

namespace dinrail::runner
{
spdlog::logger& logger()
{
    static const auto instance = dinrail::createOrGetLogger("dinrail.runner");
    return *instance;
}
} // namespace dinrail::runner
