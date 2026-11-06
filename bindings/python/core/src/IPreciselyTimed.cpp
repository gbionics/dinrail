// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>

#include <dinrail/IPreciselyTimed.h>
#include <dinrail/bindings/core/IPreciselyTimed.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

#include <chrono>
#include <cstdint>

namespace dinrail::bindings::core
{

void CreateIPreciselyTimed(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::IPreciselyTimed;
    using ::dinrail::IPreciselyTimedSimulation;
    using ::dinrail::Stamp;

    py::class_<Stamp>(module, "Stamp")
        .def(py::init<>())
        .def(py::init([](std::chrono::nanoseconds time, std::int32_t sequenceNumber) {
                 return Stamp{time, sequenceNumber};
             }),
             py::arg("time"),
             py::arg("sequenceNumber"))
        .def_readwrite("time", &Stamp::time)
        .def_readwrite("sequenceNumber", &Stamp::sequenceNumber);

    py::class_<IPreciselyTimed> preciselyTimed(module, "IPreciselyTimed");
    preciselyTimed.def("getLastInputStamp", wrapMethod(&IPreciselyTimed::getLastInputStamp));
    registerInterface(module, preciselyTimed);

    py::class_<IPreciselyTimedSimulation> simulation(module, "IPreciselyTimedSimulation");
    simulation.def("setLastInputStamp", wrapMethod(&IPreciselyTimedSimulation::setLastInputStamp));
    registerInterface(module, simulation);
}

} // namespace dinrail::bindings::core
