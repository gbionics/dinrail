// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <catch2/catch_test_macros.hpp>

#include <BipedalLocomotion/GenericContainer/Vector.h>
#include <BipedalLocomotion/ParametersHandler/IParametersHandler.h>
#include <BipedalLocomotion/ParametersHandler/YarpImplementation.h>

#include <dinrail/Parameters.h>
#include <dinrail/YarpPropertyConverter.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <yarp/os/Bottle.h>
#include <yarp/os/Time.h>
#include <yarp/os/Property.h>
#include <yarp/os/Value.h>

#include <yarp/robotinterface/Types.h>
#include <yarp/robotinterface/XMLReader.h>

namespace
{
yarp::os::Property toProperty(const yarp::robotinterface::Device& dev)
{
    using namespace yarp::robotinterface;

    ParamList params = mergeDuplicateGroups(dev.params());

    yarp::os::Property prop;
    prop.put("device", dev.type());
    prop.put("id", dev.name());

    for (const auto& param : params)
    {
        prop.fromString("(" + param.name() + " " + param.value() + ")", false);
    }

    return prop;
}

yarp::os::Property loadPropertyFromFile(const std::filesystem::path& file)
{
    yarp::os::Property yarpProperty;

    if (file.extension() == ".xml")
    {
        yarp::os::Time::useSystemClock();

        yarp::robotinterface::XMLReader reader;
        reader.setEnableDeprecated(true);
        auto result = reader.getRobotFromFile(file.string());

        // Some legacy fixtures are a raw <device> XML block instead of a
        // robotinterface <robot> root. In that case, wrap the device block.
        if (!result.parsingIsSuccessful)
        {
            std::ifstream input(file);
            REQUIRE(input.is_open());

            std::stringstream buffer;
            buffer << input.rdbuf();
            const std::string xml = buffer.str();

            const std::size_t start = xml.find("<device");
            const std::size_t endTag = xml.rfind("</device>");
            REQUIRE(start != std::string::npos);
            REQUIRE(endTag != std::string::npos);

            const std::size_t end = endTag + std::string("</device>").size();
            const std::string deviceBlock = xml.substr(start, end - start);

            const std::string wrapped = "<robot name=\"wrapped\" build=\"0\" portprefix=\"/\">\n"
                                        + deviceBlock + "\n</robot>\n";
            result = reader.getRobotFromString(wrapped);
            REQUIRE(result.parsingIsSuccessful);
        }

        const auto& devices = result.robot.devices();
        REQUIRE_FALSE(devices.empty());

        const auto& dev = devices.front();
        yarpProperty = toProperty(dev);
        return yarpProperty;
    }

    REQUIRE(yarpProperty.fromConfigFile(file.string()));
    return yarpProperty;
}

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
        if (!m_parameters.getParameter(parameterName, values))
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
        if (!m_parameters.getParameter(parameterName, values))
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
        if (!m_parameters.getParameter(parameterName, values))
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
        if (!m_parameters.getParameter(parameterName, values))
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

    const bool lhsOk = lhs.getParameter(key, lhsValues);
    const bool rhsOk = rhs.getParameter(key, rhsValues);

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

        const auto yarpGroup = yarpHandler.getGroup(key).lock();
        const auto dinrailGroup = dinrailHandler.getGroup(key).lock();

        REQUIRE(static_cast<bool>(yarpGroup));
        REQUIRE(static_cast<bool>(dinrailGroup));

        for (int i = 1; i < entry.size(); ++i)
        {
            yarp::os::Bottle* child = entry.get(i).asList();
            if (child == nullptr)
            {
                FAIL("Unexpected null sub-bottle in group entry at: " + keyPath);
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
        FAIL("Unsupported non-list YARP value type at key: " + keyPath);
    }

    yarp::os::Bottle* list = value.asList();
    REQUIRE(list != nullptr);

    if (list->size() == 0)
    {
        return;
    }

    bool allInt = true;
    bool allFloat = true;
    bool allNumeric = true;
    bool allString = true;
    bool allBool = true;
    for (int i = 0; i < list->size(); ++i)
    {
        const yarp::os::Value& element = list->get(i);
        const bool isInt = element.isInt32() || element.isInt64();
        const bool isNumeric = isInt || element.isFloat64();
        allInt = allInt && isInt;
        allFloat = allFloat && element.isFloat64();
        allNumeric = allNumeric && isNumeric;
        allString = allString && element.isString();
        allBool = allBool && element.isBool();
    }

    if (allBool)
    {
        std::vector<bool> lhsValues;
        std::vector<bool> rhsValues;


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

    if (allNumeric)
    {
        // This is a workaround as BLF parameters handler as of blf 0.24.0 does 
        // not support loading "fingersScaling          (1, 2.0, 3.5, 3.5, 3.5)"
        // as std::vector<double>, while other YARP-using codes supports it
        // (see https://github.com/gbionics/walking-teleoperation/blob/v1.3.9/modules/Utils/src/Utils.cpp#L134)
        
        // so only in this case we do not test against BLF
        std::vector<double> expectedValues;
        expectedValues.reserve(static_cast<std::size_t>(list->size()));
        for (int i = 0; i < list->size(); ++i)
        {
            expectedValues.push_back(list->get(i).asFloat64());
        }

        std::vector<double> actualValues;
        const bool rhsOk = dinrailHandler.getParameter(key, actualValues);
        INFO("Vector key path: " << keyPath);
        REQUIRE(rhsOk);
        REQUIRE(actualValues == expectedValues);
        return;
    }

    if (allString)
    {
        requireSameVector<std::string>(yarpHandler, dinrailHandler, key, keyPath);
        return;
    }

    FAIL("Unsupported list element type mix at key: " + keyPath);
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
            FAIL("Unexpected non-list element at top-level index " + std::to_string(i));
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
        yarp::os::Property yarpProperty = loadPropertyFromFile(file);

        BipedalLocomotion::ParametersHandler::YarpImplementation yarpHandler(yarpProperty);

        dinrail::Parameters params
            = dinrail::YarpPropertyConverter::toDinrailParameters(yarpProperty);
        DinrailImplementation dinrailHandler(params);

        comparePropertyHandlers(yarpProperty, yarpHandler, dinrailHandler);
    }
}

TEST_CASE("Mixed numeric fingersScaling list is converted as vector<double>",
          "[YarpPropertyConverter][BLFCompatibility]")
{
    const std::filesystem::path file
        = std::filesystem::path(DINRAIL_YARP_TEST_DATA_DIR) / "fingers-scaling-only.ini";

    yarp::os::Property yarpProperty = loadPropertyFromFile(file);

    dinrail::Parameters params = dinrail::YarpPropertyConverter::toDinrailParameters(yarpProperty);

    REQUIRE(params.check<std::vector<double>>("fingersScaling"));
    const auto& values = params.find("fingersScaling").as<std::vector<double>>();
    REQUIRE(values.size() == 5);
    REQUIRE(values[0] == 1.0);
    REQUIRE(values[1] == 2.0);
    REQUIRE(values[2] == 3.5);
    REQUIRE(values[3] == 3.5);
    REQUIRE(values[4] == 3.5);
}
