// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Status.h>

#include <sstream>
#include <type_traits>

TEST_CASE("Status defaults to OK", "[Status]")
{
    constexpr dinrail::Status status;

    STATIC_REQUIRE(status.ok());
    STATIC_REQUIRE(status.code() == dinrail::StatusCode::Ok);
    STATIC_REQUIRE(dinrail::OkStatus() == status);
}

TEST_CASE("Status contains a canonical error code", "[Status]")
{
    constexpr dinrail::Status status{dinrail::StatusCode::NotFound};

    STATIC_REQUIRE_FALSE(status.ok());
    STATIC_REQUIRE(status.code() == dinrail::StatusCode::NotFound);
    STATIC_REQUIRE(std::is_trivially_copyable_v<dinrail::Status>);
    STATIC_REQUIRE(sizeof(dinrail::Status) == sizeof(dinrail::StatusCode));
}

TEST_CASE("Status Update preserves the first error", "[Status]")
{
    dinrail::Status status;

    status.Update(dinrail::OkStatus());
    REQUIRE(status.ok());

    status.Update(dinrail::Status{dinrail::StatusCode::NotFound});
    REQUIRE(status.code() == dinrail::StatusCode::NotFound);

    status.Update(dinrail::Status{dinrail::StatusCode::Unavailable});
    REQUIRE(status.code() == dinrail::StatusCode::NotFound);

    status.Update(dinrail::OkStatus());
    REQUIRE(status.code() == dinrail::StatusCode::NotFound);
}

TEST_CASE("StatusCode converts every canonical code to its name", "[Status]")
{
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::Ok) == "Ok");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::Cancelled) == "Cancelled");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::Unknown) == "Unknown");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::InvalidArgument)
            == "InvalidArgument");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::DeadlineExceeded)
            == "DeadlineExceeded");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::NotFound) == "NotFound");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::AlreadyExists) == "AlreadyExists");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::PermissionDenied)
            == "PermissionDenied");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::ResourceExhausted)
            == "ResourceExhausted");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::FailedPrecondition)
            == "FailedPrecondition");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::Aborted) == "Aborted");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::OutOfRange) == "OutOfRange");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::Unimplemented) == "Unimplemented");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::Internal) == "Internal");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::Unavailable) == "Unavailable");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::DataLoss) == "DataLoss");
    REQUIRE(dinrail::StatusCodeToStringView(dinrail::StatusCode::Unauthenticated)
            == "Unauthenticated");
}

TEST_CASE("Status and StatusCode support string and stream formatting", "[Status]")
{
    const dinrail::Status status{dinrail::StatusCode::FailedPrecondition};
    std::ostringstream codeStream;
    std::ostringstream statusStream;

    codeStream << status.code();
    statusStream << status;

    REQUIRE(dinrail::StatusCodeToString(status.code()) == "FailedPrecondition");
    REQUIRE(status.ToStringView() == "FailedPrecondition");
    REQUIRE(status.ToString() == "FailedPrecondition");
    REQUIRE(codeStream.str() == "FailedPrecondition");
    REQUIRE(statusStream.str() == "FailedPrecondition");
}

TEST_CASE("Unknown numeric status codes format as empty strings", "[Status]")
{
    constexpr auto unknownCode = static_cast<dinrail::StatusCode>(999);
    const dinrail::Status status{unknownCode};
    std::ostringstream stream;

    stream << status;

    REQUIRE(dinrail::StatusCodeToStringView(unknownCode).empty());
    REQUIRE(dinrail::StatusCodeToString(unknownCode).empty());
    REQUIRE(status.ToStringView().empty());
    REQUIRE(status.ToString().empty());
    REQUIRE(stream.str().empty());
}
