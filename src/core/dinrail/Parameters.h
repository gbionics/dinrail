// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_PARAMETERS_H
#define DINRAIL_PARAMETERS_H

#include <chrono>
#include <string>
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
 * Furthermore, the dinrail::Parameters can also contain also other dinrail::Parameters instances nested,
 * that can be accessed with `addGroup` and `findGroup` methods.
 * 
 * In general, the `dinrail::Paramters` is designed as a way to share settings (such as encoder offset)
 * and should not to be used to share hot data like joint state, also as all values are copied, and in 
 * some cases dynamic memory is allocated (for example if a resize of a vector is called in a getParameter call).
 *
 * For this reason no multithread syncronization is implemented inside the dinrail::Parameters class,
 * so if you really have a use case for reading and writing from the same `dinrail::Parameters` instance 
 * in different threads, make sure that the read and write are protected by a mutual exclusive access mechanism
 * such as a C++'s std::mutex .
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
    void put(const std::string& key, bool value);
    /** @brief Insert or replace an integer value. */
    void put(const std::string& key, int value);
    /** @brief Insert or replace a floating-point value. */
    void put(const std::string& key, double value);
    /** @brief Insert or replace a C-string value. */
    void put(const std::string& key, const char* value);
    /** @brief Insert or replace a string value. */
    void put(const std::string& key, const std::string& value);
    /** @brief Insert or replace a duration value. */
    void put(const std::string& key, std::chrono::nanoseconds value);
    /** @brief Insert or replace a boolean vector value. */
    void put(const std::string& key, const std::vector<bool>& value);
    /** @brief Insert or replace an integer vector value. */
    void put(const std::string& key, const std::vector<int>& value);
    /** @brief Insert or replace a floating-point vector value. */
    void put(const std::string& key, const std::vector<double>& value);
    /** @brief Insert or replace a string vector value. */
    void put(const std::string& key, const std::vector<std::string>& value);
    /** @brief Insert or replace a duration vector value. */
    void put(const std::string& key, const std::vector<std::chrono::nanoseconds>& value);

    /**
     * @brief Check if a key exists either as value or as group.
     * @param key Parameter/group name.
     * @return True if found.
     */
    bool check(const std::string& key) const;

    /**
     * @brief Return value associated to key, or fallback if missing.
     * @param key Parameter name.
     * @param fallback Value returned when key is missing.
     * @param comment Optional compatibility comment (currently ignored).
     * @return Stored value if present, otherwise @p fallback.
     */
    Value
    check(const std::string& key, const Value& fallback, const std::string& comment = "") const;

    /**
     * @brief Check if a key exists and stores the requested type.
     * @tparam T Expected value type.
     * @param key Parameter name.
     * @return True if key exists and contains `T`.
     */
    template <typename T> bool check(const std::string& key) const
    {
        auto it = m_values.find(key);
        return it != m_values.end() && it->second.is<T>();
    }

    /**
     * @brief Find a scalar value by key.
     * @param key Parameter name.
     * @return Const reference to the stored value, or a null value if missing.
     */
    const Value& find(const std::string& key) const;

    /**
     * @brief Check whether this instance is the null sentinel.
     * @return True for sentinel instances returned by missing-group lookup.
     */
    bool isNull() const;

    /** @brief BLF-style getter for integer values. */
    bool getParameter(const std::string& key, int& parameter) const;
    /** @brief BLF-style getter for double values. */
    bool getParameter(const std::string& key, double& parameter) const;
    /** @brief BLF-style getter for string values. */
    bool getParameter(const std::string& key, std::string& parameter) const;
    /** @brief BLF-style getter for boolean values. */
    bool getParameter(const std::string& key, bool& parameter) const;
    /** @brief BLF-style getter for duration values. */
    bool getParameter(const std::string& key, std::chrono::nanoseconds& parameter) const;
    /** @brief BLF-style getter for boolean vector values. */
    bool getParameter(const std::string& key, std::vector<bool>& parameter) const;
    /** @brief BLF-style getter for integer vectors. */
    bool getParameter(const std::string& key, VectorProxy<int>::Ref parameter) const;
    /** @brief BLF-style getter for double vectors. */
    bool getParameter(const std::string& key, VectorProxy<double>::Ref parameter) const;
    /** @brief BLF-style getter for string vectors. */
    bool getParameter(const std::string& key, VectorProxy<std::string>::Ref parameter) const;
    /** @brief BLF-style getter for duration vectors. */
    bool getParameter(const std::string& key,
                      VectorProxy<std::chrono::nanoseconds>::Ref parameter) const;

    /** @brief BLF-style setter for integer values. */
    void setParameter(const std::string& key, const int& parameter);
    /** @brief BLF-style setter for double values. */
    void setParameter(const std::string& key, const double& parameter);
    /** @brief BLF-style setter for string values. */
    void setParameter(const std::string& key, const std::string& parameter);
    /** @brief BLF-style setter for C-string values. */
    void setParameter(const std::string& key, const char* parameter);
    /** @brief BLF-style setter for boolean values. */
    void setParameter(const std::string& key, const bool& parameter);
    /** @brief BLF-style setter for duration values. */
    void setParameter(const std::string& key, const std::chrono::nanoseconds& parameter);
    /** @brief BLF-style setter for boolean vector values. */
    void setParameter(const std::string& key, const std::vector<bool>& parameter);
    /** @brief BLF-style setter for integer vectors. */
    void setParameter(const std::string& key, const VectorProxy<const int>::Ref parameter);
    /** @brief BLF-style setter for double vectors. */
    void setParameter(const std::string& key, const VectorProxy<const double>::Ref parameter);
    /** @brief BLF-style setter for string vectors. */
    void
    setParameter(const std::string& key, const VectorProxy<const std::string>::Ref parameter);
    /** @brief BLF-style setter for duration vectors. */
    void setParameter(const std::string& key,
                      const VectorProxy<const std::chrono::nanoseconds>::Ref parameter);

    /**
     * @brief Create or retrieve a named group.
     * @param key Group name.
     * @return Mutable reference to the group.
     */
    Parameters& addGroup(const std::string& key);

    /**
     * @brief Find a named group.
     * @param key Group name.
     * @return Mutable group reference, or null sentinel if missing.
     */
    Parameters& findGroup(const std::string& key);

    /**
     * @brief Find a named group (const overload).
     * @param key Group name.
     * @return Const group reference, or null sentinel if missing.
     */
    const Parameters& findGroup(const std::string& key) const;

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
