// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_VERSION_H
#define DINRAIL_VERSION_H

#include <dinrail/Config.h>

#define DINRAIL_VERSION_AT_LEAST(major, minor, patch)                         \
    ((DINRAIL_VERSION_MAJOR > (major))                                        \
     || (DINRAIL_VERSION_MAJOR == (major) && DINRAIL_VERSION_MINOR > (minor)) \
     || (DINRAIL_VERSION_MAJOR == (major) && DINRAIL_VERSION_MINOR == (minor) \
         && DINRAIL_VERSION_PATCH >= (patch)))

#endif // DINRAIL_VERSION_H
