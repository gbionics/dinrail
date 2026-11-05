// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/IMotor.h>
#include <dinrail/bindings/core/IMotor.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

#include <cstddef>

namespace dinrail::bindings::core
{

void CreateIMotor(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::IMotor;
    using ::dinrail::IMotorSimulation;

    // Output vectors have one element per motor.
    const auto numberOfMotors = [](IMotor& self, const auto&) -> std::size_t {
        int motors = 0;
        return (self.getNumberOfMotors(&motors) && motors > 0) ? static_cast<std::size_t>(motors)
                                                               : 0;
    };

    py::class_<IMotor> motor(module, "IMotor");
    motor.def("getNumberOfMotors", wrapMethod(&IMotor::getNumberOfMotors))
        .def("getTemperature", wrapMethod(&IMotor::getTemperature))
        .def("getTemperatures", wrapMethod(&IMotor::getTemperatures, numberOfMotors))
        .def("getTemperatureLimit", wrapMethod(&IMotor::getTemperatureLimit))
        .def("setTemperatureLimit", wrapMethod(&IMotor::setTemperatureLimit))
        .def("getGearboxRatio", wrapMethod(&IMotor::getGearboxRatio))
        .def("setGearboxRatio", wrapMethod(&IMotor::setGearboxRatio));
    registerInterface(module, motor);

    py::class_<IMotorSimulation> simulation(module, "IMotorSimulation");
    simulation.def("setTemperature", wrapMethod(&IMotorSimulation::setTemperature))
        .def("setTemperatures", wrapMethod(&IMotorSimulation::setTemperatures));
    registerInterface(module, simulation);
}

} // namespace dinrail::bindings::core
