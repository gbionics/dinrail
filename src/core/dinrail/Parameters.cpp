// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/Parameters.h>

namespace dinrail
{

namespace
{

template <typename T>
const T* getTyped(const std::unordered_map<std::string, Value>& values, const std::string& key)
{
    auto it = values.find(key);
    if (it == values.end() || !it->second.is<T>())
    {
        return nullptr;
    }

    return &(it->second.as<T>());
}

template <typename T>
bool copyVectorToGenericVector(const std::vector<T>& source,
                               typename GenericVector<T>::Ref destination)
{
    if (destination.size() != source.size() && !destination.resizeVector(source.size()))
    {
        return false;
    }

    for (std::size_t i = 0; i < source.size(); ++i)
    {
        destination[i] = source[i];
    }

    return true;
}

template <typename T>
std::vector<T> copyFromGenericVector(const typename GenericVector<const T>::Ref& source)
{
    std::vector<T> output(source.size());
    for (std::size_t i = 0; i < source.size(); ++i)
    {
        output[i] = source[i];
    }

    return output;
}

} // namespace

Parameters::Parameters(bool isNull)
    : m_isNull(isNull)
{
}

void Parameters::put(const std::string& key, bool value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, int value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, double value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, const char* value)
{
    m_values.insert_or_assign(key, Value((value != nullptr) ? std::string(value) : std::string()));
}

void Parameters::put(const std::string& key, const std::string& value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, std::chrono::nanoseconds value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, const std::vector<bool>& value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, const std::vector<int>& value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, const std::vector<double>& value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, const std::vector<std::string>& value)
{
    m_values.insert_or_assign(key, Value(value));
}

void Parameters::put(const std::string& key, const std::vector<std::chrono::nanoseconds>& value)
{
    m_values.insert_or_assign(key, Value(value));
}

bool Parameters::check(const std::string& key) const
{
    return m_values.find(key) != m_values.end() || m_groups.find(key) != m_groups.end();
}

Value Parameters::check(const std::string& key,
                        const Value& fallback,
                        const std::string& comment) const
{
    static_cast<void>(comment);

    const Value& value = find(key);
    if (value.isNull())
    {
        return fallback;
    }

    return value;
}

const Value& Parameters::find(const std::string& key) const
{
    auto it = m_values.find(key);
    if (it == m_values.end())
    {
        static const Value nullValue;
        return nullValue;
    }

    return it->second;
}

bool Parameters::isNull() const
{
    return m_isNull;
}

bool Parameters::getParameter(const std::string& key, int& parameter) const
{
    const int* value = getTyped<int>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    parameter = *value;
    return true;
}

bool Parameters::getParameter(const std::string& key, double& parameter) const
{
    const double* value = getTyped<double>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    parameter = *value;
    return true;
}

bool Parameters::getParameter(const std::string& key, std::string& parameter) const
{
    const std::string* value = getTyped<std::string>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    parameter = *value;
    return true;
}

bool Parameters::getParameter(const std::string& key, bool& parameter) const
{
    const bool* value = getTyped<bool>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    parameter = *value;
    return true;
}

bool Parameters::getParameter(const std::string& key, std::chrono::nanoseconds& parameter) const
{
    const std::chrono::nanoseconds* value = getTyped<std::chrono::nanoseconds>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    parameter = *value;
    return true;
}

bool Parameters::getParameter(const std::string& key, std::vector<bool>& parameter) const
{
    const std::vector<bool>* value = getTyped<std::vector<bool>>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    parameter = *value;
    return true;
}

bool Parameters::getParameter(const std::string& key, GenericVector<int>::Ref parameter) const
{
    const std::vector<int>* value = getTyped<std::vector<int>>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    if (parameter.size() != value->size() && !parameter.resizeVector(value->size()))
    {
        return false;
    }

    for (std::size_t i = 0; i < value->size(); ++i)
    {
        parameter[i] = (*value)[i];
    }

    return true;
}

bool Parameters::getParameter(const std::string& key, GenericVector<double>::Ref parameter) const
{
    const std::vector<double>* value = getTyped<std::vector<double>>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    return copyVectorToGenericVector(*value, parameter);
}

bool Parameters::getParameter(const std::string& key,
                              GenericVector<std::string>::Ref parameter) const
{
    const std::vector<std::string>* value = getTyped<std::vector<std::string>>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    return copyVectorToGenericVector(*value, parameter);
}

bool Parameters::getParameter(const std::string& key,
                              GenericVector<std::chrono::nanoseconds>::Ref parameter) const
{
    const std::vector<std::chrono::nanoseconds>* value
        = getTyped<std::vector<std::chrono::nanoseconds>>(m_values, key);
    if (value == nullptr)
    {
        return false;
    }

    return copyVectorToGenericVector(*value, parameter);
}

void Parameters::setParameter(const std::string& key, const int& parameter)
{
    put(key, parameter);
}

void Parameters::setParameter(const std::string& key, const double& parameter)
{
    put(key, parameter);
}

void Parameters::setParameter(const std::string& key, const std::string& parameter)
{
    put(key, parameter);
}

void Parameters::setParameter(const std::string& key, const char* parameter)
{
    put(key, parameter);
}

void Parameters::setParameter(const std::string& key, const bool& parameter)
{
    put(key, parameter);
}

void Parameters::setParameter(const std::string& key, const std::chrono::nanoseconds& parameter)
{
    put(key, parameter);
}

void Parameters::setParameter(const std::string& key, const std::vector<bool>& parameter)
{
    put(key, parameter);
}

void Parameters::setParameter(const std::string& key, const GenericVector<const int>::Ref parameter)
{
    std::vector<int> converted(parameter.size());
    for (std::size_t i = 0; i < parameter.size(); ++i)
    {
        converted[i] = parameter[i];
    }

    put(key, converted);
}

void Parameters::setParameter(const std::string& key,
                              const GenericVector<const double>::Ref parameter)
{
    put(key, copyFromGenericVector<double>(parameter));
}

void Parameters::setParameter(const std::string& key,
                              const GenericVector<const std::string>::Ref parameter)
{
    put(key, copyFromGenericVector<std::string>(parameter));
}

void Parameters::setParameter(const std::string& key,
                              const GenericVector<const std::chrono::nanoseconds>::Ref parameter)
{
    put(key, copyFromGenericVector<std::chrono::nanoseconds>(parameter));
}

Parameters& Parameters::addGroup(const std::string& key)
{
    return m_groups[key];
}

Parameters& Parameters::findGroup(const std::string& key)
{
    auto it = m_groups.find(key);
    if (it == m_groups.end())
    {
        return nullGroup();
    }

    return it->second;
}

const Parameters& Parameters::findGroup(const std::string& key) const
{
    auto it = m_groups.find(key);
    if (it == m_groups.end())
    {
        return nullGroup();
    }

    return it->second;
}

Parameters& Parameters::nullGroup()
{
    static Parameters nullParameters(true);
    return nullParameters;
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
