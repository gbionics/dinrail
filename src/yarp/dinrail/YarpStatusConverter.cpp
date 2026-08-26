// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/YarpStatusConverter.h>

#include <yarp/dev/ReturnValue.h>

namespace dinrail
{

yarp::dev::ReturnValue toYarpReturnValue(const Status& status)
{
    using ReturnCode = yarp::dev::ReturnValue::return_code;

    switch (status.code())
    {
    case StatusCode::Ok:
        return ReturnCode::return_value_ok;
    case StatusCode::Unimplemented:
        return ReturnCode::return_value_error_not_implemented_by_device;
    case StatusCode::Unavailable:
        return ReturnCode::return_value_error_nws_nwc_communication_error;
    case StatusCode::FailedPrecondition:
        return ReturnCode::return_value_error_not_ready;
    case StatusCode::Internal:
        return ReturnCode::return_value_uninitialized;
    default:
        return ReturnCode::return_value_error_generic;
    }
}

Status toDinrailStatus(const yarp::dev::ReturnValue& returnValue)
{
    using ReturnCode = yarp::dev::ReturnValue::return_code;

    if (returnValue == ReturnCode::return_value_ok)
    {
        return OkStatus();
    }
    if (returnValue == ReturnCode::return_value_error_not_implemented_by_device
        || returnValue == ReturnCode::return_value_error_deprecated)
    {
        return Status{StatusCode::Unimplemented};
    }
    if (returnValue == ReturnCode::return_value_error_nws_nwc_communication_error)
    {
        return Status{StatusCode::Unavailable};
    }
    if (returnValue == ReturnCode::return_value_error_not_ready)
    {
        return Status{StatusCode::FailedPrecondition};
    }
    if (returnValue == ReturnCode::return_value_uninitialized)
    {
        return Status{StatusCode::Internal};
    }

    // Generic errors, method failures, and any future YARP error codes do not
    // provide a more specific canonical classification.
    return Status{StatusCode::Unknown};
}

} // namespace dinrail
