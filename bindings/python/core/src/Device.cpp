// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/Device.h>
#include <dinrail/Parameters.h>
#include <dinrail/RuntimeContext.h>
#include <dinrail/bindings/core/Device.h>

#include <string>

namespace dinrail::bindings::core
{

void CreateDevice(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::Device;

    // Maps the Python class of each registered interface to the callable retrieving it from a
    // Device. Interfaces are registered at import time by the modules that bind them, so dinrail
    // does not need to know them in advance.
    py::dict interfaceViewers;

    module.def(
        "registerInterface",
        [interfaceViewers](const py::type& interfaceType, const py::function& viewer) {
            interfaceViewers[interfaceType] = viewer;
        },
        py::arg("interfaceType"),
        py::arg("viewer"),
        "Register the callable used by Device.view() to retrieve interfaceType from a Device.\n\n"
        "The viewer is called with the Device and must return the interface, or None if the "
        "device does not provide it. C++ bindings should use the "
        "dinrail::bindings::core::registerInterface() helper.");

    py::class_<Device>(module, "Device")
        .def(py::init<>())
        .def(py::init<const ::dinrail::RuntimeContext&>(), py::arg("context"))
        .def("open", &Device::open, py::arg("config"), py::call_guard<py::gil_scoped_release>())
        .def("close", &Device::close, py::call_guard<py::gil_scoped_release>())
        .def("isValid", &Device::isValid)
        .def(
            "view",
            [interfaceViewers](const py::object& self,
                               const py::type& interfaceType) -> py::object {
                if (!interfaceViewers.contains(interfaceType))
                {
                    throw py::type_error(py::str(interfaceType).cast<std::string>()
                                         + " is not a registered dinrail interface. Make sure "
                                           "that the module binding it has been imported.");
                }
                return interfaceViewers[interfaceType](self);
            },
            py::arg("interfaceType"),
            "Retrieve the interface interfaceType implemented by the device, or None if it is "
            "not available.\n\n"
            "The returned interface keeps the device alive, but it is invalidated when the device "
            "is closed or opened again.");
}

} // namespace dinrail::bindings::core
