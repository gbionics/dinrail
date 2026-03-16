// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Value.h>

namespace dinrail
{

Value::Value(bool value)
    : m_value(value)
{
}

Value::Value(int value)
    : m_value(value)
{
}

Value::Value(double value)
    : m_value(value)
{
}

Value::Value(const std::string& value)
    : m_value(value)
{
}

Value::Value(std::string&& value)
    : m_value(std::move(value))
{
}

Value::Value(std::chrono::nanoseconds value)
    : m_value(value)
{
}

Value::Value(const std::vector<bool>& value)
    : m_value(value)
{
}

Value::Value(std::vector<bool>&& value)
    : m_value(std::move(value))
{
}

Value::Value(const std::vector<int>& value)
    : m_value(value)
{
}

Value::Value(std::vector<int>&& value)
    : m_value(std::move(value))
{
}

Value::Value(const std::vector<double>& value)
    : m_value(value)
{
}

Value::Value(std::vector<double>&& value)
    : m_value(std::move(value))
{
}

Value::Value(const std::vector<std::string>& value)
    : m_value(value)
{
}

Value::Value(std::vector<std::string>&& value)
    : m_value(std::move(value))
{
}

Value::Value(const std::vector<std::chrono::nanoseconds>& value)
    : m_value(value)
{
}

Value::Value(std::vector<std::chrono::nanoseconds>&& value)
    : m_value(std::move(value))
{
}

bool Value::isNull() const
{
    return std::holds_alternative<std::monostate>(m_value);
}

bool Value::isInt() const
{
    return std::holds_alternative<int>(m_value);
}

bool Value::isBool() const
{
    return std::holds_alternative<bool>(m_value);
}

bool Value::isDouble() const
{
    return std::holds_alternative<double>(m_value);
}

bool Value::isString() const
{
    return std::holds_alternative<std::string>(m_value);
}

bool Value::asBool() const
{
    if (isBool())
    {
        return as<bool>();
    }

    if (isInt())
    {
        return as<int>() != 0;
    }

    return false;
}

int Value::asInt() const
{
    if (isInt())
    {
        return as<int>();
    }

    if (isBool())
    {
        return as<bool>() ? static_cast<int>('1') : 0;
    }

    if (isDouble())
    {
        return static_cast<int>(as<double>());
    }

    return 0;
}

double Value::asDouble() const
{
    if (isDouble())
    {
        return as<double>();
    }

    if (isInt())
    {
        return static_cast<double>(as<int>());
    }

    if (isBool())
    {
        return as<bool>() ? static_cast<double>('1') : 0.0;
    }

    return 0.0;
}

std::string Value::asString() const
{
    if (isString())
    {
        return as<std::string>();
    }

    if (isBool())
    {
        return as<bool>() ? std::string("true") : std::string("false");
    }

    return std::string();
}

} // namespace dinrail
