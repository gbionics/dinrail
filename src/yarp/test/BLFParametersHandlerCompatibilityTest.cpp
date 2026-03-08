// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <BipedalLocomotion/GenericContainer/Vector.h>
#include <BipedalLocomotion/ParametersHandler/IParametersHandler.h>
#include <BipedalLocomotion/ParametersHandler/YarpImplementation.h>

#include <dinrail/Parameters.h>

#include "../YarpPropertyConverter.h"

#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/os/Value.h>

namespace
{

class DinrailImplementation : public BipedalLocomotion::ParametersHandler::IParametersHandler
{
public:
    explicit DinrailImplementation(const dinrail::Parameters& parameters)
        : m_parameters(parameters)
    {
    }

    DinrailImplementation() = default;

    bool getParameter(const std::string& parameterName, int& parameter) const override
    {
        return m_parameters.getParameter(parameterName, parameter);
    }

    bool getParameter(const std::string& parameterName, double& parameter) const override
    {
        return m_parameters.getParameter(parameterName, parameter);
    }

    bool getParameter(const std::string& parameterName, std::string& parameter) const override
    {
        return m_parameters.getParameter(parameterName, parameter);
    }

    bool getParameter(const std::string& parameterName, bool& parameter) const override
    {
        return m_parameters.getParameter(parameterName, parameter);
    }

    bool getParameter(const std::string& parameterName,
                      std::chrono::nanoseconds& parameter) const override
    {
        return m_parameters.getParameter(parameterName, parameter);
    }

    bool getParameter(const std::string& parameterName, std::vector<bool>& parameter) const override
    {
        return m_parameters.getParameter(parameterName, parameter);
    }

    bool
    getParameter(const std::string& parameterName,
                 BipedalLocomotion::GenericContainer::Vector<int>::Ref parameter) const override
    {
        std::vector<int> values;
        if (!m_parameters.getParameter(parameterName, dinrail::GenericVector<int>::Ref(values)))
        {
            return false;
        }

        if (parameter.size() != values.size() && !parameter.resizeVector(values.size()))
        {
            return false;
        }

        for (std::size_t i = 0; i < values.size(); ++i)
        {
            parameter[i] = values[i];
        }

        return true;
    }

    bool
    getParameter(const std::string& parameterName,
                 BipedalLocomotion::GenericContainer::Vector<double>::Ref parameter) const override
    {
        std::vector<double> values;
        if (!m_parameters.getParameter(parameterName, dinrail::GenericVector<double>::Ref(values)))
        {
            return false;
        }

        if (parameter.size() != values.size() && !parameter.resizeVector(values.size()))
        {
            return false;
        }

        for (std::size_t i = 0; i < values.size(); ++i)
        {
            parameter[i] = values[i];
        }

        return true;
    }

    bool getParameter(
        const std::string& parameterName,
        BipedalLocomotion::GenericContainer::Vector<std::string>::Ref parameter) const override
    {
        std::vector<std::string> values;
        if (!m_parameters.getParameter(parameterName,
                                       dinrail::GenericVector<std::string>::Ref(values)))
        {
            return false;
        }

        if (parameter.size() != values.size() && !parameter.resizeVector(values.size()))
        {
            return false;
        }

        for (std::size_t i = 0; i < values.size(); ++i)
        {
            parameter[i] = values[i];
        }

        return true;
    }

    bool getParameter(const std::string& parameterName,
                      BipedalLocomotion::GenericContainer::Vector<std::chrono::nanoseconds>::Ref
                          parameter) const override
    {
        std::vector<std::chrono::nanoseconds> values;
        if (!m_parameters.getParameter(parameterName,
                                       dinrail::GenericVector<std::chrono::nanoseconds>::Ref(
                                           values)))
        {
            return false;
        }

        if (parameter.size() != values.size() && !parameter.resizeVector(values.size()))
        {
            return false;
        }

        for (std::size_t i = 0; i < values.size(); ++i)
        {
            parameter[i] = values[i];
        }

        return true;
    }

    void setParameter(const std::string& parameterName, const int& parameter) override
    {
        m_parameters.setParameter(parameterName, parameter);
    }

    void setParameter(const std::string& parameterName, const double& parameter) override
    {
        m_parameters.setParameter(parameterName, parameter);
    }

    void setParameter(const std::string& parameterName, const std::string& parameter) override
    {
        m_parameters.setParameter(parameterName, parameter);
    }

    void setParameter(const std::string& parameterName, const char* parameter) override
    {
        m_parameters.setParameter(parameterName, parameter);
    }

    void setParameter(const std::string& parameterName, const bool& parameter) override
    {
        m_parameters.setParameter(parameterName, parameter);
    }

    void setParameter(const std::string& parameterName,
                      const std::chrono::nanoseconds& parameter) override
    {
        m_parameters.setParameter(parameterName, parameter);
    }

    void setParameter(const std::string& parameterName, const std::vector<bool>& parameter) override
    {
        m_parameters.setParameter(parameterName, parameter);
    }

    void setParameter(
        const std::string& parameterName,
        const BipedalLocomotion::GenericContainer::Vector<const int>::Ref parameter) override
    {
        std::vector<int> values(parameter.size());
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            values[i] = parameter[i];
        }
        m_parameters.put(parameterName, values);
    }

    void setParameter(
        const std::string& parameterName,
        const BipedalLocomotion::GenericContainer::Vector<const double>::Ref parameter) override
    {
        std::vector<double> values(parameter.size());
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            values[i] = parameter[i];
        }
        m_parameters.put(parameterName, values);
    }

    void setParameter(const std::string& parameterName,
                      const BipedalLocomotion::GenericContainer::Vector<const std::string>::Ref
                          parameter) override
    {
        std::vector<std::string> values(parameter.size());
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            values[i] = parameter[i];
        }
        m_parameters.put(parameterName, values);
    }

    void setParameter(
        const std::string& parameterName,
        const BipedalLocomotion::GenericContainer::Vector<const std::chrono::nanoseconds>::Ref
            parameter) override
    {
        std::vector<std::chrono::nanoseconds> values(parameter.size());
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            values[i] = parameter[i];
        }
        m_parameters.put(parameterName, values);
    }

    weak_ptr getGroup(const std::string& name) const override
    {
        const auto& group = m_parameters.findGroup(name);
        if (group.isNull())
        {
            return weak_ptr{};
        }

        auto it = m_groupsCache.find(name);
        if (it == m_groupsCache.end())
        {
            auto inserted
                = m_groupsCache.emplace(name, std::make_shared<DinrailImplementation>(group));
            it = inserted.first;
        }

        return it->second;
    }

    bool setGroup(const std::string& name, shared_ptr newGroup) override
    {
        auto casted = std::dynamic_pointer_cast<DinrailImplementation>(newGroup);
        if (casted == nullptr)
        {
            return false;
        }

        m_parameters.addGroup(name) = casted->m_parameters;
        m_groupsCache.erase(name);
        return true;
    }

    std::string toString() const override
    {
        return "DinrailImplementation";
    }

    bool isEmpty() const override
    {
        return m_parameters.getValueKeys().empty() && m_parameters.getGroupKeys().empty();
    }

    void clear() override
    {
        m_parameters.clear();
        m_groupsCache.clear();
    }

    shared_ptr clone() const override
    {
        return std::make_shared<DinrailImplementation>(m_parameters);
    }

private:
    dinrail::Parameters m_parameters;
    mutable std::unordered_map<std::string, std::shared_ptr<DinrailImplementation>> m_groupsCache;
};

bool isGroupEntry(const yarp::os::Bottle& entry)
{
    if (entry.size() < 2)
    {
        return false;
    }

    for (int i = 1; i < entry.size(); ++i)
    {
        if (!entry.get(i).isList())
        {
            return false;
        }
        yarp::os::Bottle* child = entry.get(i).asList();
        if (child == nullptr || child->size() < 2 || !child->get(0).isString())
        {
            return false;
        }
    }

    return true;
}

template <typename T>
void requireSameScalar(const BipedalLocomotion::ParametersHandler::IParametersHandler& lhs,
                       const BipedalLocomotion::ParametersHandler::IParametersHandler& rhs,
                       const std::string& key,
                       const std::string& path)
{
    T lhsValue{};
    T rhsValue{};

    const bool lhsOk = lhs.getParameter(key, lhsValue);
    const bool rhsOk = rhs.getParameter(key, rhsValue);

    std::cout << "[BLFCompatibility] Scalar parameter tested: " << path
              << " (type=" << typeid(T).name() << ")" << std::endl;
    INFO("Scalar key path: " << path);

    REQUIRE(lhsOk == rhsOk);
    if (lhsOk)
    {
        REQUIRE(lhsValue == rhsValue);
    }
}

template <typename T>
void requireSameVector(const BipedalLocomotion::ParametersHandler::IParametersHandler& lhs,
                       const BipedalLocomotion::ParametersHandler::IParametersHandler& rhs,
                       const std::string& key,
                       const std::string& path)
{
    std::vector<T> lhsValues;
    std::vector<T> rhsValues;

    typename BipedalLocomotion::GenericContainer::Vector<T>::Ref lhsRef(lhsValues);
    typename BipedalLocomotion::GenericContainer::Vector<T>::Ref rhsRef(rhsValues);

    const bool lhsOk = lhs.getParameter(key, lhsRef);
    const bool rhsOk = rhs.getParameter(key, rhsRef);

    std::cout << "[BLFCompatibility] Vector parameter tested: " << path
              << " (type=" << typeid(T).name() << ")" << std::endl;
    INFO("Vector key path: " << path);
    REQUIRE(lhsOk == rhsOk);
    if (lhsOk)
    {
        REQUIRE(lhsValues == rhsValues);
    }
}

void compareEntry(const yarp::os::Bottle& entry,
                  const BipedalLocomotion::ParametersHandler::IParametersHandler& yarpHandler,
                  const BipedalLocomotion::ParametersHandler::IParametersHandler& dinrailHandler,
                  const std::string& parentPath = "")
{
    if (entry.size() < 2 || !entry.get(0).isString())
    {
        return;
    }

    const std::string key = entry.get(0).asString();
    const std::string keyPath = parentPath.empty() ? key : parentPath + "/" + key;

    if (isGroupEntry(entry))
    {
        std::cout << "[BLFCompatibility] Group tested: " << keyPath << std::endl;
        INFO("Group key path: " << keyPath);

        const auto yarpGroup = yarpHandler.getGroup(key).lock();
        const auto dinrailGroup = dinrailHandler.getGroup(key).lock();

        REQUIRE(static_cast<bool>(yarpGroup));
        REQUIRE(static_cast<bool>(dinrailGroup));

        for (int i = 1; i < entry.size(); ++i)
        {
            yarp::os::Bottle* child = entry.get(i).asList();
            if (child == nullptr)
            {
                continue;
            }
            compareEntry(*child, *yarpGroup, *dinrailGroup, keyPath);
        }

        return;
    }

    const yarp::os::Value& value = entry.get(1);
    if (value.isBool())
    {
        requireSameScalar<bool>(yarpHandler, dinrailHandler, key, keyPath);
        return;
    }

    if (value.isInt32() || value.isInt64())
    {
        requireSameScalar<int>(yarpHandler, dinrailHandler, key, keyPath);
        return;
    }

    if (value.isFloat64())
    {
        requireSameScalar<double>(yarpHandler, dinrailHandler, key, keyPath);
        return;
    }

    if (value.isString())
    {
        requireSameScalar<std::string>(yarpHandler, dinrailHandler, key, keyPath);
        return;
    }

    if (!value.isList())
    {
        return;
    }

    yarp::os::Bottle* list = value.asList();
    REQUIRE(list != nullptr);

    bool allInt = true;
    bool allFloat = true;
    bool allString = true;
    bool allBool = true;
    for (int i = 0; i < list->size(); ++i)
    {
        const yarp::os::Value& element = list->get(i);
        const bool isInt = element.isInt32() || element.isInt64();
        allInt = allInt && isInt;
        allFloat = allFloat && element.isFloat64();
        allString = allString && element.isString();
        allBool = allBool && element.isBool();
    }

    if (allBool)
    {
        std::vector<bool> lhsValues;
        std::vector<bool> rhsValues;

        std::cout << "[BLFCompatibility] Vector parameter tested: " << keyPath
                  << " (type=bool)" << std::endl;
        INFO("Vector key path: " << keyPath);

        const bool lhsOk = yarpHandler.getParameter(key, lhsValues);
        const bool rhsOk = dinrailHandler.getParameter(key, rhsValues);
        REQUIRE(lhsOk == rhsOk);
        if (lhsOk)
        {
            REQUIRE(lhsValues == rhsValues);
        }
        return;
    }

    if (allInt)
    {
        requireSameVector<int>(yarpHandler, dinrailHandler, key, keyPath);
        return;
    }

    if (allFloat)
    {
        requireSameVector<double>(yarpHandler, dinrailHandler, key, keyPath);
        return;
    }

    if (allString)
    {
        requireSameVector<std::string>(yarpHandler, dinrailHandler, key, keyPath);
    }
}

void comparePropertyHandlers(
    const yarp::os::Property& yarpProp,
    const BipedalLocomotion::ParametersHandler::IParametersHandler& yarpHandler,
    const BipedalLocomotion::ParametersHandler::IParametersHandler& dinrailHandler)
{
    yarp::os::Bottle root;
    root.fromString(yarpProp.toString());

    for (int i = 0; i < root.size(); ++i)
    {
        if (!root.get(i).isList())
        {
            continue;
        }

        yarp::os::Bottle* entry = root.get(i).asList();
        REQUIRE(entry != nullptr);
        compareEntry(*entry, yarpHandler, dinrailHandler);
    }
}

} // namespace

TEST_CASE("BLF YarpImplementation and DinrailImplementation are compatible on sample files",
          "[YarpPropertyConverter][BLFCompatibility]")
{
    const std::vector<std::filesystem::path> files = {
        std::filesystem::path(DINRAIL_YARP_TEST_DATA_DIR) / "openXRjoypad.ini",
        std::filesystem::path(DINRAIL_YARP_TEST_DATA_DIR) / "robot-dynamics-estimator.xml",
        std::filesystem::path(DINRAIL_YARP_TEST_DATA_DIR) / "qpIKBLF.ini",
    };

    for (const auto& file : files)
    {
        INFO("Checking file: " << file.string());

        yarp::os::Property yarpProperty;
        REQUIRE(yarpProperty.fromConfigFile(file.string()));

        BipedalLocomotion::ParametersHandler::YarpImplementation yarpHandler(yarpProperty);

        dinrail::Parameters params
            = dinrail::YarpPropertyConverter::toDinrailParameters(yarpProperty);
        DinrailImplementation dinrailHandler(params);

        comparePropertyHandlers(yarpProperty, yarpHandler, dinrailHandler);
    }
}
