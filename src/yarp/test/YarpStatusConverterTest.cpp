// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/YarpStatusConverter.h>

#include <yarp/dev/ReturnValue.h>

namespace
{
using ReturnCode = yarp::dev::ReturnValue::return_code;

void requireConversion(ReturnCode yarpCode, dinrail::StatusCode dinrailCode)
{
    const yarp::dev::ReturnValue yarpValue{yarpCode};
    REQUIRE(dinrail::toDinrailStatus(yarpValue).code() == dinrailCode);
}
} // namespace

TEST_CASE("YARP return values convert to canonical dinrail statuses", "[YarpStatusConverter]")
{
    requireConversion(ReturnCode::return_value_ok, dinrail::StatusCode::Ok);
    requireConversion(ReturnCode::return_value_error_generic, dinrail::StatusCode::Unknown);
    requireConversion(ReturnCode::return_value_error_not_implemented_by_device,
                      dinrail::StatusCode::Unimplemented);
    requireConversion(ReturnCode::return_value_error_nws_nwc_communication_error,
                      dinrail::StatusCode::Unavailable);
    requireConversion(ReturnCode::return_value_error_deprecated,
                      dinrail::StatusCode::Unimplemented);
    requireConversion(ReturnCode::return_value_error_method_failed, dinrail::StatusCode::Unknown);
    requireConversion(ReturnCode::return_value_error_not_ready,
                      dinrail::StatusCode::FailedPrecondition);
    requireConversion(ReturnCode::return_value_uninitialized, dinrail::StatusCode::Internal);
}

TEST_CASE("Canonical dinrail statuses convert to YARP return values", "[YarpStatusConverter]")
{
    REQUIRE(dinrail::toYarpReturnValue(dinrail::OkStatus()) == ReturnCode::return_value_ok);
    REQUIRE(dinrail::toYarpReturnValue(dinrail::Status{dinrail::StatusCode::Unknown})
            == ReturnCode::return_value_error_generic);
    REQUIRE(dinrail::toYarpReturnValue(dinrail::Status{dinrail::StatusCode::Unimplemented})
            == ReturnCode::return_value_error_not_implemented_by_device);
    REQUIRE(dinrail::toYarpReturnValue(dinrail::Status{dinrail::StatusCode::Unavailable})
            == ReturnCode::return_value_error_nws_nwc_communication_error);
    REQUIRE(dinrail::toYarpReturnValue(dinrail::Status{dinrail::StatusCode::FailedPrecondition})
            == ReturnCode::return_value_error_not_ready);
    REQUIRE(dinrail::toYarpReturnValue(dinrail::Status{dinrail::StatusCode::Internal})
            == ReturnCode::return_value_uninitialized);
}

TEST_CASE("Statuses without a YARP equivalent become generic errors", "[YarpStatusConverter]")
{
    REQUIRE(dinrail::toYarpReturnValue(dinrail::Status{dinrail::StatusCode::InvalidArgument})
            == ReturnCode::return_value_error_generic);
    REQUIRE(dinrail::toYarpReturnValue(dinrail::Status{dinrail::StatusCode::DataLoss})
            == ReturnCode::return_value_error_generic);
}
