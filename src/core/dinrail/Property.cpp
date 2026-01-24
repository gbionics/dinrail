// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Property.h>

namespace dinrail
{

Property::Property() = default;

Property::~Property() = default;

Property::Property(const Property& other)
    : m_data(other.m_data)
{
    for (const auto& [key, group] : other.m_groups)
    {
        m_groups[key] = std::make_unique<Property>(*group);
    }
}

Property& Property::operator=(const Property& other)
{
    if (this != &other)
    {
        m_data = other.m_data;
        m_groups.clear();
        for (const auto& [key, group] : other.m_groups)
        {
            m_groups[key] = std::make_unique<Property>(*group);
        }
    }
    return *this;
}

Property::Property(Property&& other) noexcept = default;

Property& Property::operator=(Property&& other) noexcept = default;

void Property::put(const std::string& key, const std::string& value)
{
    m_data[key] = value;
}

void Property::put(const std::string& key, int value)
{
    m_data[key] = value;
}

void Property::put(const std::string& key, double value)
{
    m_data[key] = value;
}

Property& Property::addGroup(const std::string& key)
{
    // Match YARP behavior: always create a new group, clearing any existing one
    m_groups[key] = std::make_unique<Property>();
    return *m_groups[key];
}

bool Property::check(const std::string& key) const
{
    return m_data.find(key) != m_data.end() || m_groups.find(key) != m_groups.end();
}

std::string Property::get(const std::string& key) const
{
    auto it = m_data.find(key);
    if (it != m_data.end())
    {
        // Try to get as string
        if (std::holds_alternative<std::string>(it->second))
        {
            return std::get<std::string>(it->second);
        }
    }
    return "";
}

int Property::getInt(const std::string& key) const
{
    auto it = m_data.find(key);
    if (it != m_data.end())
    {
        if (std::holds_alternative<int>(it->second))
        {
            return std::get<int>(it->second);
        }
    }
    return 0;
}

const Property* Property::findGroup(const std::string& key) const
{
    auto it = m_groups.find(key);
    if (it != m_groups.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void Property::clear()
{
    m_data.clear();
    m_groups.clear();
}

} // namespace dinrail
