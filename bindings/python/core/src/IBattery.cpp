// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/IBattery.h>
#include <dinrail/bindings/core/IBattery.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

namespace dinrail::bindings::core
{

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
    battery.def("getBatteryVoltage", wrapMethod(&IBattery::getBatteryVoltage))
        .def("getBatteryCurrent", wrapMethod(&IBattery::getBatteryCurrent))
        .def("getBatteryCharge", wrapMethod(&IBattery::getBatteryCharge))
        .def("getBatteryStatus", wrapMethod(&IBattery::getBatteryStatus))
        .def("getBatteryTemperature", wrapMethod(&IBattery::getBatteryTemperature))
        .def("getBatteryInfo", wrapMethod(&IBattery::getBatteryInfo));
    registerInterface(module, battery);

    py::class_<IBatterySimulation> simulation(module, "IBatterySimulation");
    simulation.def("setBatteryVoltage", wrapMethod(&IBatterySimulation::setBatteryVoltage))
        .def("setBatteryCurrent", wrapMethod(&IBatterySimulation::setBatteryCurrent))
        .def("setBatteryCharge", wrapMethod(&IBatterySimulation::setBatteryCharge))
        .def("setBatteryStatus", wrapMethod(&IBatterySimulation::setBatteryStatus))
        .def("setBatteryTemperature", wrapMethod(&IBatterySimulation::setBatteryTemperature))
        .def("setBatteryInfo", wrapMethod(&IBatterySimulation::setBatteryInfo));
    registerInterface(module, simulation);
}

} // namespace dinrail::bindings::core
