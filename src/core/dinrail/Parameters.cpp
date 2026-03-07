// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Parameters.h>

namespace dinrail
{

void Parameters::put(const std::string& key, bool value)
{
    m_values[key] = value;
}

void Parameters::put(const std::string& key, int value)
{
    m_values[key] = static_cast<std::int64_t>(value);
}

void Parameters::put(const std::string& key, std::int64_t value)
{
    m_values[key] = value;
}

void Parameters::put(const std::string& key, double value)
{
    m_values[key] = value;
}

void Parameters::put(const std::string& key, const char* value)
{
    m_values[key] = (value != nullptr) ? std::string(value) : std::string();
}

void Parameters::put(const std::string& key, const std::string& value)
{
    m_values[key] = value;
}

bool Parameters::check(const std::string& key) const
{
    return m_values.find(key) != m_values.end() || m_groups.find(key) != m_groups.end();
}

std::optional<bool> Parameters::getBool(const std::string& key) const
{
    auto it = m_values.find(key);
    if (it == m_values.end() || !std::holds_alternative<bool>(it->second))
    {
        return std::nullopt;
    }

    return std::get<bool>(it->second);
}

std::optional<std::int64_t> Parameters::getInt64(const std::string& key) const
{
    auto it = m_values.find(key);
    if (it == m_values.end() || !std::holds_alternative<std::int64_t>(it->second))
    {
        return std::nullopt;
    }

    return std::get<std::int64_t>(it->second);
}

std::optional<double> Parameters::getFloat64(const std::string& key) const
{
    auto it = m_values.find(key);
    if (it == m_values.end() || !std::holds_alternative<double>(it->second))
    {
        return std::nullopt;
    }

    return std::get<double>(it->second);
}

std::optional<std::string> Parameters::getString(const std::string& key) const
{
    auto it = m_values.find(key);
    if (it == m_values.end() || !std::holds_alternative<std::string>(it->second))
    {
        return std::nullopt;
    }

    return std::get<std::string>(it->second);
}

Parameters& Parameters::addGroup(const std::string& key)
{
    return m_groups[key];
}

std::optional<std::reference_wrapper<Parameters>> Parameters::findGroup(const std::string& key)
{
    auto it = m_groups.find(key);
    if (it == m_groups.end())
    {
        return std::nullopt;
    }

    return std::ref(it->second);
}

std::optional<std::reference_wrapper<const Parameters>> Parameters::findGroup(const std::string& key) const
{
    auto it = m_groups.find(key);
    if (it == m_groups.end())
    {
        return std::nullopt;
    }

    return std::cref(it->second);
}

std::optional<std::reference_wrapper<const Parameters::Value>> Parameters::get(const std::string& key) const
{
    auto it = m_values.find(key);
    if (it == m_values.end())
    {
        return std::nullopt;
    }

    return std::cref(it->second);
}

std::vector<std::string> Parameters::getValueKeys() const
{
    std::vector<std::string> keys;
    keys.reserve(m_values.size());
    for (const auto& [key, value] : m_values)
    {
        static_cast<void>(value);
        keys.push_back(key);
    }
    return keys;
}

std::vector<std::string> Parameters::getGroupKeys() const
{
    std::vector<std::string> keys;
    keys.reserve(m_groups.size());
    for (const auto& [key, value] : m_groups)
    {
        static_cast<void>(value);
        keys.push_back(key);
    }
    return keys;
}

void Parameters::clear()
{
    m_values.clear();
    m_groups.clear();
}

} // namespace dinrail
