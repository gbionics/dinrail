// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/IAxisInfo.h>
#include <dinrail/bindings/core/IAxisInfo.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

namespace dinrail::bindings::core
{

void CreateIAxisInfo(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::IAxisInfo;
    using ::dinrail::JointType;

    py::enum_<JointType>(module, "JointType")
        .value("REVOLUTE", JointType::REVOLUTE)
        .value("PRISMATIC", JointType::PRISMATIC)
        .value("UNKNOWN", JointType::UNKNOWN);

    py::class_<IAxisInfo> axisInfo(module, "IAxisInfo");
    axisInfo.def("getAxes", wrapMethod(&IAxisInfo::getAxes))
        .def("getAxisName", wrapMethod(&IAxisInfo::getAxisName))
        .def("getJointType", wrapMethod(&IAxisInfo::getJointType));
    registerInterface(module, axisInfo);
}

} // namespace dinrail::bindings::core
