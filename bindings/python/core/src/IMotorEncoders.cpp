// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/IMotorEncoders.h>
#include <dinrail/bindings/core/IMotorEncoders.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

#include <cstddef>

namespace dinrail::bindings::core
{

void CreateIMotorEncoders(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::IMotorEncoders;
    using ::dinrail::IMotorEncodersSimulation;

    // Output vectors have one element per motor encoder.
    const auto numberOfEncoders = [](IMotorEncoders& self, const auto&) -> std::size_t {
        int encoders = 0;
        return (self.getNumberOfMotorEncoders(&encoders) && encoders > 0)
                   ? static_cast<std::size_t>(encoders)
                   : 0;
    };

    py::class_<IMotorEncoders> encoders(module, "IMotorEncoders");
    encoders.def("getNumberOfMotorEncoders", wrapMethod(&IMotorEncoders::getNumberOfMotorEncoders))
        .def("getMotorEncoderCountsPerRevolution",
             wrapMethod(&IMotorEncoders::getMotorEncoderCountsPerRevolution))
        .def("getMotorEncoder", wrapMethod(&IMotorEncoders::getMotorEncoder))
        .def("getMotorEncoders", wrapMethod(&IMotorEncoders::getMotorEncoders, numberOfEncoders))
        .def("getMotorEncoderTimed", wrapMethod(&IMotorEncoders::getMotorEncoderTimed))
        .def("getMotorEncodersTimed",
             wrapMethod(&IMotorEncoders::getMotorEncodersTimed, numberOfEncoders))
        .def("getMotorEncoderSpeed", wrapMethod(&IMotorEncoders::getMotorEncoderSpeed))
        .def("getMotorEncoderSpeeds",
             wrapMethod(&IMotorEncoders::getMotorEncoderSpeeds, numberOfEncoders))
        .def("getMotorEncoderAcceleration",
             wrapMethod(&IMotorEncoders::getMotorEncoderAcceleration))
        .def("getMotorEncoderAccelerations",
             wrapMethod(&IMotorEncoders::getMotorEncoderAccelerations, numberOfEncoders));
    registerInterface(module, encoders);

    py::class_<IMotorEncodersSimulation> simulation(module, "IMotorEncodersSimulation");
    simulation.def("setMotorEncoder", wrapMethod(&IMotorEncodersSimulation::setMotorEncoder))
        .def("setMotorEncoders", wrapMethod(&IMotorEncodersSimulation::setMotorEncoders))
        .def("setMotorEncoderTimed", wrapMethod(&IMotorEncodersSimulation::setMotorEncoderTimed))
        .def("setMotorEncodersTimed", wrapMethod(&IMotorEncodersSimulation::setMotorEncodersTimed))
        .def("setMotorEncoderSpeed", wrapMethod(&IMotorEncodersSimulation::setMotorEncoderSpeed))
        .def("setMotorEncoderSpeeds", wrapMethod(&IMotorEncodersSimulation::setMotorEncoderSpeeds))
        .def("setMotorEncoderAcceleration",
             wrapMethod(&IMotorEncodersSimulation::setMotorEncoderAcceleration))
        .def("setMotorEncoderAccelerations",
             wrapMethod(&IMotorEncodersSimulation::setMotorEncoderAccelerations));
    registerInterface(module, simulation);
}

} // namespace dinrail::bindings::core
