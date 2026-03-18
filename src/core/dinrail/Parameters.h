// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_PARAMETERS_H
#define DINRAIL_PARAMETERS_H

#include <chrono>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <dinrail/VectorProxy.h>
#include <dinrail/Value.h>

namespace dinrail
{

/**
 * @brief Hierarchical parameter container for dinrail device configuration.
 *
 * The dinrail::Parameters is a key-value storage that can store values of the following types:
 * * bool (`bool`)
 * * int (`int`)
 * * double (`double`)
 * * string (`std::string`)
 * * duration (`std::chrono::nanoseconds`)
 * * vector of bool
 * * vector of int
 * * vector of double
 * * vector of string
 * * vector of duration
 *
 * Furthermore, dinrail::Parameters can also contain other nested dinrail::Parameters instances,
 * which can be accessed with the `addGroup` and `findGroup` methods.
 * 
 * In general, `dinrail::Parameters` is designed as a way to share settings (such as encoder offset)
 * and should not be used to share hot data like joint state, as all values are copied and, in 
 * some cases, dynamic memory is allocated (for example, if a resize of a vector is triggered by a
 * getParameter call).
 *
 * For this reason no multithread synchronization is implemented inside the dinrail::Parameters class,
 * so if you really have a use case for reading and writing from the same `dinrail::Parameters` instance 
 * in different threads, make sure that the reads and writes are protected by a mutually exclusive access
 * mechanism such as the C++ `std::mutex` class.
 * 
 * For more details on the dinrail::Parameters class, check the
 * docs/parameters.md
 */
class Parameters
{
public:
    Parameters() = default;
    ~Parameters() = default;
    Parameters(const Parameters&) = default;
    Parameters& operator=(const Parameters&) = default;
    Parameters(Parameters&&) noexcept = default;
    Parameters& operator=(Parameters&&) noexcept = default;

    /** @brief Insert or replace a boolean value. */
    void put(std::string_view key, bool value);
    /** @brief Insert or replace an integer value. */
    void put(std::string_view key, int value);
    /** @brief Insert or replace a floating-point value. */
    void put(std::string_view key, double value);
    /** @brief Insert or replace a C-string value. */
    void put(std::string_view key, const char* value);
    /** @brief Insert or replace a string value. */
    void put(std::string_view key, const std::string& value);
    /** @brief Insert or replace a duration value. */
    void put(std::string_view key, std::chrono::nanoseconds value);
    /** @brief Insert or replace a boolean vector value. */
    void put(std::string_view key, const std::vector<bool>& value);
    /** @brief Insert or replace an integer vector value. */
    void put(std::string_view key, const VectorProxy<const int>::Ref value);
    /** @brief Insert or replace a floating-point vector value. */
    void put(std::string_view key, const VectorProxy<const double>::Ref value);
    /** @brief Insert or replace a string vector value. */
    void put(std::string_view key, const VectorProxy<const std::string>::Ref value);
    /** @brief Insert or replace a duration vector value. */
    void put(std::string_view key,
             const VectorProxy<const std::chrono::nanoseconds>::Ref value);

    /**
     * @brief Check if a key exists either as value or as group.
     * @param key Parameter/group name.
     * @return True if found.
     */
    bool check(std::string_view key) const;

    /**
     * @brief Return value associated to key, or fallback if missing.
     * @param key Parameter name.
     * @param fallback Value returned when key is missing.
     * @param comment Optional compatibility comment (currently ignored).
     * @return Stored value if present, otherwise @p fallback.
     */
    Value
    check(std::string_view key, const Value& fallback, const std::string& comment = "") const;

    /**
     * @brief Check if a key exists and stores the requested type.
     * @tparam T Expected value type.
     * @param key Parameter name.
     * @return True if key exists and contains `T`.
     */
    template <typename T> bool check(std::string_view key) const
    {
        auto it = m_values.find(std::string(key));
        return it != m_values.end() && it->second.is<T>();
    }

    /**
     * @brief Find a scalar value by key.
     * @param key Parameter name.
     * @return Const reference to the stored value, or a null value if missing.
     */
    const Value& find(std::string_view key) const;

    /**
     * @brief Check whether this instance is the null sentinel.
     * @return True for sentinel instances returned by missing-group lookup.
     */
    bool isNull() const;

    /** @brief BLF-style getter for integer values. */
    bool getParameter(std::string_view key, int& parameter) const;
    /** @brief BLF-style getter for double values. */
    bool getParameter(std::string_view key, double& parameter) const;
    /** @brief BLF-style getter for string values. */
    bool getParameter(std::string_view key, std::string& parameter) const;
    /** @brief BLF-style getter for boolean values. */
    bool getParameter(std::string_view key, bool& parameter) const;
    /** @brief BLF-style getter for duration values. */
    bool getParameter(std::string_view key, std::chrono::nanoseconds& parameter) const;
    /** @brief BLF-style getter for boolean vector values. */
    bool getParameter(std::string_view key, std::vector<bool>& parameter) const;
    /** @brief BLF-style getter for integer vectors. */
    bool getParameter(std::string_view key, VectorProxy<int>::Ref parameter) const;
    /** @brief BLF-style getter for double vectors. */
    bool getParameter(std::string_view key, VectorProxy<double>::Ref parameter) const;
    /** @brief BLF-style getter for string vectors. */
    bool getParameter(std::string_view key, VectorProxy<std::string>::Ref parameter) const;
    /** @brief BLF-style getter for duration vectors. */
    bool getParameter(std::string_view key,
                      VectorProxy<std::chrono::nanoseconds>::Ref parameter) const;

    /** @brief BLF-style setter for integer values. */
    void setParameter(std::string_view key, const int& parameter);
    /** @brief BLF-style setter for double values. */
    void setParameter(std::string_view key, const double& parameter);
    /** @brief BLF-style setter for string values. */
    void setParameter(std::string_view key, const std::string& parameter);
    /** @brief BLF-style setter for C-string values. */
    void setParameter(std::string_view key, const char* parameter);
    /** @brief BLF-style setter for boolean values. */
    void setParameter(std::string_view key, const bool& parameter);
    /** @brief BLF-style setter for duration values. */
    void setParameter(std::string_view key, const std::chrono::nanoseconds& parameter);
    /** @brief BLF-style setter for boolean vector values. */
    void setParameter(std::string_view key, const std::vector<bool>& parameter);
    /** @brief BLF-style setter for integer vectors. */
    void setParameter(std::string_view key, const VectorProxy<const int>::Ref parameter);
    /** @brief BLF-style setter for double vectors. */
    void setParameter(std::string_view key, const VectorProxy<const double>::Ref parameter);
    /** @brief BLF-style setter for string vectors. */
    void
    setParameter(std::string_view key, const VectorProxy<const std::string>::Ref parameter);
    /** @brief BLF-style setter for duration vectors. */
    void setParameter(std::string_view key,
                      const VectorProxy<const std::chrono::nanoseconds>::Ref parameter);

    /**
     * @brief Create or retrieve a named group.
     * @param key Group name.
     * @return Mutable reference to the group.
     */
    Parameters& addGroup(std::string_view key);

    /**
     * @brief Find a named group.
     * @param key Group name.
     * @return Mutable group reference, or null sentinel if missing.
     */
    Parameters& findGroup(std::string_view key);

    /**
     * @brief Find a named group (const overload).
     * @param key Group name.
     * @return Const group reference, or null sentinel if missing.
     */
    const Parameters& findGroup(std::string_view key) const;

    /**
     * @brief List all value keys at this level.
     * @return Vector of key names.
     */
    std::vector<std::string> getValueKeys() const;

    /**
     * @brief List all group keys at this level.
     * @return Vector of group names.
     */
    std::vector<std::string> getGroupKeys() const;

    /**
     * @brief Remove all values and groups from this instance.
     */
    void clear();

private:
    explicit Parameters(bool isNull);

    static Parameters& nullGroup();

    bool m_isNull{false};
    std::unordered_map<std::string, Value> m_values;
    std::unordered_map<std::string, Parameters> m_groups;
};

} // namespace dinrail

#endif // DINRAIL_PARAMETERS_H
