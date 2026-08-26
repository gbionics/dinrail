// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <dinrail/Status.h>

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
