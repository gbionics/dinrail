// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_VALUE_H
#define DINRAIL_VALUE_H

#include <chrono>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace dinrail
{

/**
 * @brief Typed value container used by @ref dinrail::Parameters.
 *
 * This class stores scalar and vector configuration values, plus a null
 * state (represented by `std::monostate`) for missing entries.
 */
class Value
{
public:
    /**
     * @brief Variant type backing the stored value.
     */
    using Variant = std::variant<std::monostate,
                                 bool,
                                 int,
                                 double,
                                 std::string,
                                 std::chrono::nanoseconds,
                                 std::vector<bool>,
                                 std::vector<int>,
                                 std::vector<double>,
                                 std::vector<std::string>,
                                 std::vector<std::chrono::nanoseconds>>;

    /**
     * @brief Construct a null value.
     */
    Value() = default;
    ~Value() = default;
    Value(const Value&) = default;
    Value& operator=(const Value&) = default;
    Value(Value&&) noexcept = default;
    Value& operator=(Value&&) noexcept = default;

    /** @brief Construct from a boolean scalar. */
    explicit Value(bool value);
    /** @brief Construct from an integer scalar. */
    explicit Value(int value);
    /** @brief Construct from a double scalar. */
    explicit Value(double value);
    /** @brief Construct from a string scalar. */
    explicit Value(const std::string& value);
    /** @brief Construct from a moved string scalar. */
    explicit Value(std::string&& value);
    /** @brief Construct from a duration scalar. */
    explicit Value(std::chrono::nanoseconds value);
    /** @brief Construct from a boolean vector. */
    explicit Value(const std::vector<bool>& value);
    /** @brief Construct from a moved boolean vector. */
    explicit Value(std::vector<bool>&& value);
    /** @brief Construct from an integer vector. */
    explicit Value(const std::vector<int>& value);
    /** @brief Construct from a moved integer vector. */
    explicit Value(std::vector<int>&& value);
    /** @brief Construct from a double vector. */
    explicit Value(const std::vector<double>& value);
    /** @brief Construct from a moved double vector. */
    explicit Value(std::vector<double>&& value);
    /** @brief Construct from a string vector. */
    explicit Value(const std::vector<std::string>& value);
    /** @brief Construct from a moved string vector. */
    explicit Value(std::vector<std::string>&& value);
    /** @brief Construct from a duration vector. */
    explicit Value(const std::vector<std::chrono::nanoseconds>& value);
    /** @brief Construct from a moved duration vector. */
    explicit Value(std::vector<std::chrono::nanoseconds>&& value);

    /**
     * @brief Check whether the stored type is `T`.
     * @tparam T Type to probe in the underlying variant.
     * @return True if the stored value currently holds `T`.
     */
    template <typename T> bool is() const
    {
        return std::holds_alternative<T>(m_value);
    }

    /**
     * @brief Access the stored value as `T` (const).
     * @tparam T Expected stored type.
     * @return Const reference to the stored value.
     * @throws std::bad_variant_access If the stored type is not `T`.
     */
    template <typename T> const T& as() const
    {
        return std::get<T>(m_value);
    }

    /**
     * @brief Access the stored value as `T` (mutable).
     * @tparam T Expected stored type.
     * @return Mutable reference to the stored value.
     * @throws std::bad_variant_access If the stored type is not `T`.
     */
    template <typename T> T& as()
    {
        return std::get<T>(m_value);
    }

    /** @brief Check if this value is null (`std::monostate`). */
    bool isNull() const;
    /** @brief Check if this value stores an `int`. */
    bool isInt() const;
    /** @brief Check if this value stores a `bool`. */
    bool isBool() const;
    /** @brief Check if this value stores a `double`. */
    bool isDouble() const;
    /** @brief Check if this value stores a `std::string`. */
    bool isString() const;

    /**
     * @brief Convert to bool using YARP-like scalar compatibility.
     * @return Converted boolean value.
     */
    bool asBool() const;
    /**
     * @brief Convert to int using YARP-like scalar compatibility.
     * @return Converted integer value.
     */
    int asInt() const;
    /**
     * @brief Convert to double using YARP-like scalar compatibility.
     * @return Converted floating-point value.
     */
    double asDouble() const;
    /**
     * @brief Convert to string using YARP-like scalar compatibility.
     * @return Converted string value.
     */
    std::string asString() const;

private:
    Variant m_value;
};

} // namespace dinrail

#endif // DINRAIL_VALUE_H
