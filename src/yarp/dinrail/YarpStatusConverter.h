// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_YARPSTATUSCONVERTER_H
#define DINRAIL_YARPSTATUSCONVERTER_H

#include <dinrail/Status.h>

namespace yarp::dev
{
class ReturnValue;
}

namespace dinrail
{

/**
 * @brief Convert a dinrail status to its closest YARP representation.
 *
 * The codes are mapped as follows:
 *
 * | dinrail status code              | YARP return code                                      |
 * | -------------------------------- | ----------------------------------------------------- |
 * | `StatusCode::Ok`                 | `return_value_ok`                                     |
 * | `StatusCode::Unimplemented`      | `return_value_error_not_implemented_by_device`        |
 * | `StatusCode::Unavailable`        | `return_value_error_nws_nwc_communication_error`      |
 * | `StatusCode::FailedPrecondition` | `return_value_error_not_ready`                        |
 * | `StatusCode::Internal`           | `return_value_uninitialized`                          |
 * | Any other code                   | `return_value_error_generic`                          |
 *
 * Since YARP has fewer error codes, this conversion is lossy. In particular,
 * all dinrail codes in the last row become Unknown when converted back to a
 * dinrail::Status.
 */
yarp::dev::ReturnValue toYarpReturnValue(const Status& status);

/**
 * @brief Convert a YARP return value to its canonical dinrail representation.
 *
 * The codes are mapped as follows:
 *
 * | YARP return code                                    | dinrail status code              |
 * | --------------------------------------------------- | -------------------------------- |
 * | `return_value_ok`                                   | `StatusCode::Ok`                 |
 * | `return_value_error_generic`                        | `StatusCode::Unknown`            |
 * | `return_value_error_not_implemented_by_device`      | `StatusCode::Unimplemented`      |
 * | `return_value_error_nws_nwc_communication_error`    | `StatusCode::Unavailable`        |
 * | `return_value_error_deprecated`                     | `StatusCode::Unimplemented`      |
 * | `return_value_error_method_failed`                  | `StatusCode::Unknown`            |
 * | `return_value_error_not_ready`                      | `StatusCode::FailedPrecondition` |
 * | `return_value_uninitialized`                        | `StatusCode::Internal`           |
 * | Any future code                                     | `StatusCode::Unknown`            |
 */
Status toDinrailStatus(const yarp::dev::ReturnValue& returnValue);

} // namespace dinrail

#endif // DINRAIL_YARPSTATUSCONVERTER_H
