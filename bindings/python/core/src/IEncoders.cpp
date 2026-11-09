// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/IEncoders.h>
#include <dinrail/bindings/core/IEncoders.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

#include <cstddef>

namespace dinrail::bindings::core
{

void CreateIEncoders(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::IEncoders;
    using ::dinrail::IEncodersSimulation;

    // Output vectors have one element per axis.
    const auto numberOfAxes = [](IEncoders& self, const auto&) -> std::size_t {
        int axes = 0;
        return (self.getAxes(&axes) && axes > 0) ? static_cast<std::size_t>(axes) : 0;
    };

    py::class_<IEncoders> encoders(module, "IEncoders");
    encoders.def("getAxes", wrapMethod(&IEncoders::getAxes))
        .def("getEncoder", wrapMethod(&IEncoders::getEncoder))
        .def("getEncoders", wrapMethod(&IEncoders::getEncoders, numberOfAxes))
        .def("getEncoderTimed", wrapMethod(&IEncoders::getEncoderTimed))
        .def("getEncodersTimed", wrapMethod(&IEncoders::getEncodersTimed, numberOfAxes))
        .def("getEncoderSpeed", wrapMethod(&IEncoders::getEncoderSpeed))
        .def("getEncoderSpeeds", wrapMethod(&IEncoders::getEncoderSpeeds, numberOfAxes))
        .def("getEncoderAcceleration", wrapMethod(&IEncoders::getEncoderAcceleration))
        .def("getEncoderAccelerations",
             wrapMethod(&IEncoders::getEncoderAccelerations, numberOfAxes));
    registerInterface(module, encoders);

    py::class_<IEncodersSimulation> simulation(module, "IEncodersSimulation");
    simulation.def("setEncoder", wrapMethod(&IEncodersSimulation::setEncoder))
        .def("setEncoders", wrapMethod(&IEncodersSimulation::setEncoders))
        .def("setEncoderTimed", wrapMethod(&IEncodersSimulation::setEncoderTimed))
        .def("setEncodersTimed", wrapMethod(&IEncodersSimulation::setEncodersTimed))
        .def("setEncoderSpeed", wrapMethod(&IEncodersSimulation::setEncoderSpeed))
        .def("setEncoderSpeeds", wrapMethod(&IEncodersSimulation::setEncoderSpeeds))
        .def("setEncoderAcceleration", wrapMethod(&IEncodersSimulation::setEncoderAcceleration))
        .def("setEncoderAccelerations", wrapMethod(&IEncodersSimulation::setEncoderAccelerations));
    registerInterface(module, simulation);
}

} // namespace dinrail::bindings::core
