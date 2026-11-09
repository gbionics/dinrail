// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/IJoypadControl.h>
#include <dinrail/bindings/core/IJoypadControl.h>
#include <dinrail/bindings/core/InterfaceRegistry.h>
#include <dinrail/bindings/core/MethodWrapper.h>

namespace dinrail::bindings::core
{

void CreateIJoypadControl(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::IJoypadControl;
    using ::dinrail::JoypadDeviceEvent;

    py::enum_<JoypadDeviceEvent>(module, "JoypadDeviceEvent")
        .value("NoEvent", JoypadDeviceEvent::NoEvent)
        .value("Connected", JoypadDeviceEvent::Connected)
        .value("Disconnected", JoypadDeviceEvent::Disconnected)
        .value("UpdateComplete", JoypadDeviceEvent::UpdateComplete);

    py::class_<IJoypadControl> joypad(module, "IJoypadControl");
    joypad.def("getAxisCount", wrapMethod(&IJoypadControl::getAxisCount))
        .def("getButtonCount", wrapMethod(&IJoypadControl::getButtonCount))
        .def("getHatCount", wrapMethod(&IJoypadControl::getHatCount))
        .def("getAxis", wrapMethod(&IJoypadControl::getAxis))
        .def("getButton", wrapMethod(&IJoypadControl::getButton))
        .def("getHat", wrapMethod(&IJoypadControl::getHat))
        .def("reconnect", wrapMethod(&IJoypadControl::reconnect))
        .def("getLastEvent", wrapMethod(&IJoypadControl::getLastEvent));
    registerInterface(module, joypad);
}

} // namespace dinrail::bindings::core
