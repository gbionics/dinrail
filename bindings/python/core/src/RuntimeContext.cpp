// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <dinrail/PluginUtils.h>
#include <dinrail/RuntimeContext.h>
#include <dinrail/bindings/core/RuntimeContext.h>

namespace dinrail::bindings::core
{

void CreateRuntimeContext(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::AvailableDevices;
    using ::dinrail::DeviceInfo;
    using ::dinrail::InteropDevices;
    using ::dinrail::InteropPluginInfo;
    using ::dinrail::RuntimeContext;

    py::class_<DeviceInfo>(module, "DeviceInfo")
        .def_readonly("name", &DeviceInfo::name)
        .def_readonly("location", &DeviceInfo::location);

    py::class_<InteropPluginInfo>(module, "InteropPluginInfo")
        .def_readonly("name", &InteropPluginInfo::name)
        .def_readonly("location", &InteropPluginInfo::location);

    py::class_<InteropDevices>(module, "InteropDevices")
        .def_readonly("interopPlugin", &InteropDevices::interopPlugin)
        .def_readonly("devices", &InteropDevices::devices);

    py::class_<AvailableDevices>(module, "AvailableDevices")
        .def_readonly("nativeDevices", &AvailableDevices::nativeDevices)
        .def_readonly("interopDevices", &AvailableDevices::interopDevices);

    py::class_<RuntimeContext>(module, "RuntimeContext")
        .def(py::init<>())
        .def_static("getDefault", []() { return RuntimeContext::getDefault(); })
        .def("listNativeDevices",
             &RuntimeContext::listNativeDevices,
             py::call_guard<py::gil_scoped_release>())
        .def("listInteropPlugins",
             &RuntimeContext::listInteropPlugins,
             py::call_guard<py::gil_scoped_release>())
        .def("listInteropDevices",
             &RuntimeContext::listInteropDevices,
             py::call_guard<py::gil_scoped_release>())
        .def("listDevices", &RuntimeContext::listDevices, py::call_guard<py::gil_scoped_release>());
}

} // namespace dinrail::bindings::core
