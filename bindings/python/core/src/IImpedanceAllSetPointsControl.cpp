// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <dinrail/IImpedanceAllSetPointsControl.h>
#include <dinrail/bindings/core/IImpedanceAllSetPointsControl.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

#include <cstddef>
#include <tuple>
#include <vector>

namespace dinrail::bindings::core
{

void CreateIImpedanceAllSetPointsControl(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::IImpedanceAllSetPointsControl;
    using IntVector = ::dinrail::VectorProxy<const int>::Ref;
    using InVector = ::dinrail::VectorProxy<const double>::Ref;
    using OutVector = ::dinrail::VectorProxy<double>::Ref;

    // The output vectors have one element per requested joint.
    const auto numberOfRequestedJoints
        = [](auto&, const auto& inputs) -> std::size_t { return std::get<0>(inputs).size(); };

    py::class_<IImpedanceAllSetPointsControl> control(module, "IImpedanceAllSetPointsControl");
    control.def("setSetPoint", wrapMethod(&IImpedanceAllSetPointsControl::setSetPoint))
        .def("setSetPoints",
             wrapMethod(static_cast<bool (IImpedanceAllSetPointsControl::*)(
                            IntVector, InVector, InVector, InVector, InVector, InVector)>(
                 &IImpedanceAllSetPointsControl::setSetPoints)))
        .def("setSetPoints",
             wrapMethod(static_cast<bool (IImpedanceAllSetPointsControl::*)(InVector,
                                                                            InVector,
                                                                            InVector,
                                                                            InVector,
                                                                            InVector)>(
                 &IImpedanceAllSetPointsControl::setSetPoints)))
        .def("getSetPoint", wrapMethod(&IImpedanceAllSetPointsControl::getSetPoint))
        .def("getSetPoints",
             wrapMethod(static_cast<bool (IImpedanceAllSetPointsControl::*)(
                            IntVector, OutVector, OutVector, OutVector, OutVector, OutVector)>(
                            &IImpedanceAllSetPointsControl::getSetPoints),
                        numberOfRequestedJoints))
        .def(
            "getSetPoints",
            [](IImpedanceAllSetPointsControl& self, std::size_t numberOfJoints) {
                std::vector<double> pos(numberOfJoints);
                std::vector<double> vel(numberOfJoints);
                std::vector<double> torque(numberOfJoints);
                std::vector<double> stiffness(numberOfJoints);
                std::vector<double> damping(numberOfJoints);
                bool ok = false;
                {
                    py::gil_scoped_release release;
                    ok = self.getSetPoints(OutVector(pos),
                                           OutVector(vel),
                                           OutVector(torque),
                                           OutVector(stiffness),
                                           OutVector(damping));
                }
                return std::make_tuple(ok,
                                       std::move(pos),
                                       std::move(vel),
                                       std::move(torque),
                                       std::move(stiffness),
                                       std::move(damping));
            },
            py::arg("numberOfJoints"),
            "Get the set points of all the controlled joints, that are numberOfJoints.");
    registerInterface(module, control);
}

} // namespace dinrail::bindings::core
