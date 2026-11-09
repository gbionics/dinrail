// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <dinrail/Parameters.h>
#include <dinrail/Value.h>
#include <dinrail/bindings/core/Parameters.h>

#include <chrono>
#include <string>
#include <vector>

namespace dinrail::bindings::core
{

namespace
{

namespace py = ::pybind11;

bool isBool(const py::handle& value)
{
    return py::isinstance<py::bool_>(value);
}

// Python bool is a subclass of int, so it is explicitly excluded.
bool isInt(const py::handle& value)
{
    return py::isinstance<py::int_>(value) && !isBool(value);
}

bool isTimedelta(const py::handle& value)
{
    const py::object timedelta = py::module_::import("datetime").attr("timedelta");
    return py::isinstance(value, timedelta);
}

std::string typeName(const py::handle& value)
{
    return py::type::of(value).attr("__name__").cast<std::string>();
}

template <class T> std::vector<T> toVector(const py::sequence& sequence)
{
    std::vector<T> output;
    output.reserve(sequence.size());
    for (const auto& item : sequence)
    {
        output.push_back(item.cast<T>());
    }
    return output;
}

// Conversion rules match the ones used to convert YARP lists to dinrail::Parameters.
void putSequence(::dinrail::Parameters& parameters,
                 const std::string& key,
                 const py::sequence& sequence)
{
    if (sequence.size() == 0)
    {
        parameters.put(key, std::vector<std::string>{});
        return;
    }

    bool allBool = true;
    bool allInt = true;
    bool allNumeric = true;
    bool allString = true;
    bool allTimedelta = true;
    for (const auto& item : sequence)
    {
        allBool = allBool && isBool(item);
        allInt = allInt && isInt(item);
        allNumeric = allNumeric && (isInt(item) || py::isinstance<py::float_>(item));
        allString = allString && py::isinstance<py::str>(item);
        allTimedelta = allTimedelta && isTimedelta(item);
    }

    if (allBool)
    {
        parameters.put(key, toVector<bool>(sequence));
    } else if (allInt)
    {
        parameters.put(key, toVector<int>(sequence));
    } else if (allNumeric)
    {
        parameters.put(key, toVector<double>(sequence));
    } else if (allString)
    {
        parameters.put(key, toVector<std::string>(sequence));
    } else if (allTimedelta)
    {
        parameters.put(key, toVector<std::chrono::nanoseconds>(sequence));
    } else
    {
        throw py::type_error("Unsupported mixed list type for dinrail parameter '" + key + "'.");
    }
}

void putValue(::dinrail::Parameters& parameters, const std::string& key, const py::handle& value)
{
    if (isBool(value))
    {
        parameters.put(key, value.cast<bool>());
    } else if (isInt(value))
    {
        parameters.put(key, value.cast<int>());
    } else if (py::isinstance<py::float_>(value))
    {
        parameters.put(key, value.cast<double>());
    } else if (py::isinstance<py::str>(value))
    {
        parameters.put(key, value.cast<std::string>());
    } else if (isTimedelta(value))
    {
        parameters.put(key, value.cast<std::chrono::nanoseconds>());
    } else if (py::isinstance<py::list>(value) || py::isinstance<py::tuple>(value))
    {
        putSequence(parameters, key, value.cast<py::sequence>());
    } else
    {
        throw py::type_error("Unsupported type '" + typeName(value) + "' for dinrail parameter '"
                             + key + "'.");
    }
}

// Nested dictionaries are stored as groups.
void putDict(::dinrail::Parameters& parameters, const py::dict& dict)
{
    for (const auto& item : dict)
    {
        if (!py::isinstance<py::str>(item.first))
        {
            throw py::type_error("dinrail parameter keys must be strings, got '"
                                 + typeName(item.first) + "'.");
        }

        const auto key = item.first.cast<std::string>();
        if (py::isinstance<py::dict>(item.second))
        {
            putDict(parameters.addGroup(key), item.second.cast<py::dict>());
        } else
        {
            putValue(parameters, key, item.second);
        }
    }
}

py::object toPython(const ::dinrail::Value& value)
{
    if (value.is<bool>())
    {
        return py::cast(value.as<bool>());
    }
    if (value.is<int>())
    {
        return py::cast(value.as<int>());
    }
    if (value.is<double>())
    {
        return py::cast(value.as<double>());
    }
    if (value.is<std::string>())
    {
        return py::cast(value.as<std::string>());
    }
    if (value.is<std::chrono::nanoseconds>())
    {
        return py::cast(value.as<std::chrono::nanoseconds>());
    }
    if (value.is<std::vector<bool>>())
    {
        return py::cast(value.as<std::vector<bool>>());
    }
    if (value.is<std::vector<int>>())
    {
        return py::cast(value.as<std::vector<int>>());
    }
    if (value.is<std::vector<double>>())
    {
        return py::cast(value.as<std::vector<double>>());
    }
    if (value.is<std::vector<std::string>>())
    {
        return py::cast(value.as<std::vector<std::string>>());
    }
    if (value.is<std::vector<std::chrono::nanoseconds>>())
    {
        return py::cast(value.as<std::vector<std::chrono::nanoseconds>>());
    }
    return py::none();
}

} // namespace

void CreateParameters(pybind11::module_& module)
{
    using ::dinrail::Parameters;

    py::class_<Parameters>(module, "Parameters")
        .def(py::init<>())
        .def(py::init([](const py::dict& values) {
                 Parameters parameters;
                 putDict(parameters, values);
                 return parameters;
             }),
             py::arg("values"),
             "Create the parameters from a dict, where nested dicts are stored as groups.")
        .def(
            "put",
            [](Parameters& self, const std::string& key, const py::object& value) {
                putValue(self, key, value);
            },
            py::arg("key"),
            py::arg("value"))
        .def(
            "check",
            [](const Parameters& self, const std::string& key) { return self.check(key); },
            py::arg("key"))
        .def(
            "find",
            [](const Parameters& self, const std::string& key) { return toPython(self.find(key)); },
            py::arg("key"),
            "Return the value associated to key, or None if it is missing.")
        .def("isNull", &Parameters::isNull)
        .def(
            "addGroup",
            [](Parameters& self, const std::string& key) -> Parameters& {
                return self.addGroup(key);
            },
            py::arg("key"),
            py::return_value_policy::reference_internal)
        .def(
            "findGroup",
            [](Parameters& self, const std::string& key) -> Parameters* {
                Parameters& group = self.findGroup(key);
                return group.isNull() ? nullptr : &group;
            },
            py::arg("key"),
            py::return_value_policy::reference_internal,
            "Return the group associated to key, or None if it is missing.")
        .def("getValueKeys", &Parameters::getValueKeys)
        .def("getGroupKeys", &Parameters::getGroupKeys)
        .def("clear", &Parameters::clear);

    // Allow passing a dict wherever dinrail::Parameters is expected, e.g. Device.open().
    py::implicitly_convertible<py::dict, Parameters>();
}

} // namespace dinrail::bindings::core
