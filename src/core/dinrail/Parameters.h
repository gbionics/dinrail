// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_PARAMETERS_H
#define DINRAIL_PARAMETERS_H

#include <functional>
#include <optional>
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace dinrail
{

/**
 * @brief Hierarchical parameter container for dinrail device configuration.
 *
 * Parameters stores typed key-value pairs and named nested groups.
 */
class Parameters
{
public:
    using Value = std::variant<bool, std::int64_t, double, std::string>;

    Parameters() = default;
    ~Parameters() = default;
    Parameters(const Parameters&) = default;
    Parameters& operator=(const Parameters&) = default;
    Parameters(Parameters&&) noexcept = default;
    Parameters& operator=(Parameters&&) noexcept = default;

    void put(const std::string& key, bool value);
    void put(const std::string& key, int value);
    void put(const std::string& key, std::int64_t value);
    void put(const std::string& key, double value);
    void put(const std::string& key, const char* value);
    void put(const std::string& key, const std::string& value);

    bool check(const std::string& key) const;

    std::optional<bool> getBool(const std::string& key) const;
    std::optional<std::int64_t> getInt64(const std::string& key) const;
    std::optional<double> getFloat64(const std::string& key) const;
    std::optional<std::string> getString(const std::string& key) const;

    template <typename T>
    bool check(const std::string& key) const
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            return getBool(key).has_value();
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
            return getInt64(key).has_value();
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            return getFloat64(key).has_value();
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return getString(key).has_value();
        }
        else
        {
            static_assert(!sizeof(T*), "Unsupported type for Parameters::check<T>");
        }
    }

    Parameters& addGroup(const std::string& key);
    std::optional<std::reference_wrapper<Parameters>> findGroup(const std::string& key);
    std::optional<std::reference_wrapper<const Parameters>> findGroup(const std::string& key) const;

    std::optional<std::reference_wrapper<const Value>> get(const std::string& key) const;

    std::vector<std::string> getValueKeys() const;
    std::vector<std::string> getGroupKeys() const;

    void clear();

private:
    std::unordered_map<std::string, Value> m_values;
    std::unordered_map<std::string, Parameters> m_groups;
};

} // namespace dinrail

#endif // DINRAIL_PARAMETERS_H
