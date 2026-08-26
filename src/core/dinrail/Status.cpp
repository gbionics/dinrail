// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Status.h>

#include <ostream>

namespace dinrail
{

std::string_view StatusCodeToStringView(StatusCode code) noexcept
{
    switch (code)
    {
    case StatusCode::Ok:
        return "Ok";
    case StatusCode::Cancelled:
        return "Cancelled";
    case StatusCode::Unknown:
        return "Unknown";
    case StatusCode::InvalidArgument:
        return "InvalidArgument";
    case StatusCode::DeadlineExceeded:
        return "DeadlineExceeded";
    case StatusCode::NotFound:
        return "NotFound";
    case StatusCode::AlreadyExists:
        return "AlreadyExists";
    case StatusCode::PermissionDenied:
        return "PermissionDenied";
    case StatusCode::ResourceExhausted:
        return "ResourceExhausted";
    case StatusCode::FailedPrecondition:
        return "FailedPrecondition";
    case StatusCode::Aborted:
        return "Aborted";
    case StatusCode::OutOfRange:
        return "OutOfRange";
    case StatusCode::Unimplemented:
        return "Unimplemented";
    case StatusCode::Internal:
        return "Internal";
    case StatusCode::Unavailable:
        return "Unavailable";
    case StatusCode::DataLoss:
        return "DataLoss";
    case StatusCode::Unauthenticated:
        return "Unauthenticated";
    default:
        return {};
    }
}

std::string StatusCodeToString(StatusCode code)
{
    return std::string{StatusCodeToStringView(code)};
}

std::ostream& operator<<(std::ostream& os, StatusCode code)
{
    return os << StatusCodeToStringView(code);
}

std::string_view Status::ToStringView() const noexcept
{
    return StatusCodeToStringView(code());
}

std::string Status::ToString() const
{
    return StatusCodeToString(code());
}

std::ostream& operator<<(std::ostream& os, const Status& status)
{
    return os << status.code();
}

} // namespace dinrail
