// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_STATUS_H
#define DINRAIL_STATUS_H

namespace dinrail
{

/**
 * @brief Canonical outcome codes for operations returning dinrail::Status.
 *
 * The semantic and numeric values match the canonical Google RPC/gRPC/Abseil status codes
 * defined by google.rpc.Code, see:
 * * https://github.com/googleapis/googleapis/blob/master/google/rpc/code.proto
 * * https://grpc.github.io/grpc/core/md_doc_statuscodes.html
 * * https://abseil.io/docs/cpp/guides/status
 */
enum class StatusCode : int
{
    Ok = 0,
    Cancelled = 1,
    Unknown = 2,
    InvalidArgument = 3,
    DeadlineExceeded = 4,
    NotFound = 5,
    AlreadyExists = 6,
    PermissionDenied = 7,
    ResourceExhausted = 8,
    FailedPrecondition = 9,
    Aborted = 10,
    OutOfRange = 11,
    Unimplemented = 12,
    Internal = 13,
    Unavailable = 14,
    DataLoss = 15,
    Unauthenticated = 16,
};

/**
 * @brief Lightweight outcome of an operation.
 *
 * Status stores only a canonical StatusCode. It intentionally has no error
 * message or payload support.
 */
class [[nodiscard]] Status final
{
public:
    /** @brief Construct an OK status. */
    constexpr Status() noexcept = default;

    /** @brief Construct a status containing @p code. */
    constexpr explicit Status(StatusCode code) noexcept
        : m_code(code)
    {
    }

    /** @brief Return true when this status represents success. */
    [[nodiscard]] constexpr bool ok() const noexcept
    {
        return m_code == StatusCode::Ok;
    }

    /** @brief Return the canonical status code. */
    [[nodiscard]] constexpr StatusCode code() const noexcept
    {
        return m_code;
    }

    /**
     * @brief Record @p newStatus if this status is currently OK.
     *
     * This implements first-error-wins aggregation: after the first non-OK
     * status is recorded, subsequent updates have no effect.
     */
    constexpr void Update(const Status& newStatus) noexcept
    {
        if (ok())
        {
            *this = newStatus;
        }
    }

    /** @brief Explicitly acknowledge that this status is intentionally ignored. */
    constexpr void IgnoreError() const noexcept
    {
    }

    friend constexpr bool operator==(const Status&, const Status&) noexcept = default;

private:
    StatusCode m_code{StatusCode::Ok};
};

/** @brief Return an OK status. */
[[nodiscard]] constexpr Status OkStatus() noexcept
{
    return Status{};
}

} // namespace dinrail

#endif // DINRAIL_STATUS_H
