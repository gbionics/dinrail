// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_COMMONADAPTERS_H
#define DINRAIL_COMMONADAPTERS_H

#include <dinrail/IPreciselyTimed.h>
#include <dinrail/InterfaceAdapter.h>
#include <dinrail/YarpStampConverter.h>
#include <yarp/dev/IPreciselyTimed.h>

namespace dinrail
{

template <>
class InterfaceAdapter<yarp::dev::IPreciselyTimed, dinrail::IPreciselyTimed>
    : public dinrail::InterfaceAdapterBase<yarp::dev::IPreciselyTimed, dinrail::IPreciselyTimed>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    yarp::os::Stamp getLastInputStamp() override;
};

template <>
class InterfaceAdapter<dinrail::IPreciselyTimed, yarp::dev::IPreciselyTimed>
    : public InterfaceAdapterBase<dinrail::IPreciselyTimed, yarp::dev::IPreciselyTimed>
{
public:
    using InterfaceAdapterBase::InterfaceAdapterBase;
    Stamp getLastInputStamp() override;
};

} // namespace dinrail

#endif // DINRAIL_COMMONADAPTERS_H
