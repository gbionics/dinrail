// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/IJointFault.h>
#include <dinrail/bindings/core/IJointFault.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

namespace dinrail::bindings::core
{

void CreateIJointFault(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::IJointFault;
    using ::dinrail::IJointFaultSimulation;

    py::class_<IJointFault> jointFault(module, "IJointFault");
    jointFault.def("getLastJointFault", wrapMethod(&IJointFault::getLastJointFault));
    registerInterface(module, jointFault);

    py::class_<IJointFaultSimulation> simulation(module, "IJointFaultSimulation");
    simulation.def("setLastJointFault", wrapMethod(&IJointFaultSimulation::setLastJointFault));
    registerInterface(module, simulation);
}

} // namespace dinrail::bindings::core
