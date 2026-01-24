// SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_PROPERTY_H
#define DINRAIL_PROPERTY_H

#include <map>
#include <memory>
#include <string>
#include <variant>

namespace dinrail
{

/**
 * @brief A simple property container for device configuration.
 *
 * This class provides a key-value store for configuring devices,
 * similar to YARP's Property class.
 */
class Property
{
public:
    using Value = std::variant<int, double, std::string>;

    /**
     * @brief Default constructor.
     */
    Property();

    /**
     * @brief Destructor.
     */
    ~Property();

    /**
     * @brief Copy constructor.
     */
    Property(const Property& other);

    /**
     * @brief Copy assignment operator.
     */
    Property& operator=(const Property& other);

    /**
     * @brief Move constructor.
     */
    Property(Property&& other) noexcept;

    /**
     * @brief Move assignment operator.
     */
    Property& operator=(Property&& other) noexcept;

    /**
     * @brief Associate the given key with the given string.
     * @param key The property key.
     * @param value The string value.
     */
    void put(const std::string& key, const std::string& value);

    /**
     * @brief Associate the given key with the given integer.
     * @param key The property key.
     * @param value The integer value.
     */
    void put(const std::string& key, int value);

    /**
     * @brief Associate the given key with the given floating point number.
     * @param key The property key.
     * @param value The floating point value.
     */
    void put(const std::string& key, double value);

    /**
     * @brief Add a nested group.
     *
     * @warning the group object returned is valid only until the Property
     * object is destroyed or the group is removed.
     *
     * @param key the key
     * @return the nested group, represented as a Property
     */
    Property& addGroup(const std::string& key);

    /**
     * @brief Check if a key exists.
     * @param key The property key.
     * @return true if the key exists, false otherwise.
     */
    bool check(const std::string& key) const;

    /**
     * @brief Get a string value from the property.
     * @param key The property key.
     * @return the string value, or empty string if not found
     */
    std::string get(const std::string& key) const;

    /**
     * @brief Get an integer value from the property.
     * @param key The property key.
     * @return the integer value, or 0 if not found or not an integer
     */
    int getInt(const std::string& key) const;

    /**
     * @brief Find a nested group.
     * @param key The group name.
     * @return pointer to the nested Property, or nullptr if not found
     */
    const Property* findGroup(const std::string& key) const;

    /**
     * @brief Remove all associations.
     */
    void clear();

private:
    std::map<std::string, Value> m_data;
    std::map<std::string, std::unique_ptr<Property>> m_groups;
};

} // namespace dinrail

#endif // DINRAIL_PROPERTY_H
