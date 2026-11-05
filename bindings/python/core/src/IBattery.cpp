// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/IBattery.h>
#include <dinrail/Status.h>
#include <dinrail/bindings/core/IBattery.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>

#include <string>
#include <tuple>

namespace dinrail::bindings::core
{

namespace
{

// Bind a getter with an output argument as a method returning a (Status, value) tuple.
template <class T, class Interface> auto bindGetter(::dinrail::Status (Interface::*getter)(T&))
{
    return [getter](Interface& self) {
        T value{};
        const ::dinrail::Status status = (self.*getter)(value);
        return std::make_tuple(status, value);
    };
}

} // namespace

void CreateIBattery(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::BatteryStatus;
    using ::dinrail::IBattery;
    using ::dinrail::IBatterySimulation;

    py::enum_<BatteryStatus>(module, "BatteryStatus")
        .value("OkStandby", BatteryStatus::OkStandby)
        .value("OkInCharge", BatteryStatus::OkInCharge)
        .value("OkInUse", BatteryStatus::OkInUse)
        .value("GeneralError", BatteryStatus::GeneralError)
        .value("Timeout", BatteryStatus::Timeout)
        .value("LowWarning", BatteryStatus::LowWarning)
        .value("CriticalWarning", BatteryStatus::CriticalWarning);

    py::class_<IBattery> battery(module, "IBattery");
    battery
        .def("getBatteryVoltage",
             bindGetter(&IBattery::getBatteryVoltage),
             py::call_guard<py::gil_scoped_release>())
        .def("getBatteryCurrent",
             bindGetter(&IBattery::getBatteryCurrent),
             py::call_guard<py::gil_scoped_release>())
        .def("getBatteryCharge",
             bindGetter(&IBattery::getBatteryCharge),
             py::call_guard<py::gil_scoped_release>())
        .def("getBatteryStatus",
             bindGetter(&IBattery::getBatteryStatus),
             py::call_guard<py::gil_scoped_release>())
        .def("getBatteryTemperature",
             bindGetter(&IBattery::getBatteryTemperature),
             py::call_guard<py::gil_scoped_release>())
        .def("getBatteryInfo",
             bindGetter(&IBattery::getBatteryInfo),
             py::call_guard<py::gil_scoped_release>());
    registerInterface(module, battery);

    py::class_<IBatterySimulation> batterySimulation(module, "IBatterySimulation");
    batterySimulation
        .def("setBatteryVoltage",
             &IBatterySimulation::setBatteryVoltage,
             py::arg("voltage"),
             py::call_guard<py::gil_scoped_release>())
        .def("setBatteryCurrent",
             &IBatterySimulation::setBatteryCurrent,
             py::arg("current"),
             py::call_guard<py::gil_scoped_release>())
        .def("setBatteryCharge",
             &IBatterySimulation::setBatteryCharge,
             py::arg("charge"),
             py::call_guard<py::gil_scoped_release>())
        .def("setBatteryStatus",
             &IBatterySimulation::setBatteryStatus,
             py::arg("status"),
             py::call_guard<py::gil_scoped_release>())
        .def("setBatteryTemperature",
             &IBatterySimulation::setBatteryTemperature,
             py::arg("temperature"),
             py::call_guard<py::gil_scoped_release>())
        .def("setBatteryInfo",
             &IBatterySimulation::setBatteryInfo,
             py::arg("batteryInfo"),
             py::call_guard<py::gil_scoped_release>());
    registerInterface(module, batterySimulation);
}

} // namespace dinrail::bindings::core
