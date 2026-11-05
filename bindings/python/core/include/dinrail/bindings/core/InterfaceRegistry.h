// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_BINDINGS_CORE_INTERFACEREGISTRY_H
#define DINRAIL_BINDINGS_CORE_INTERFACEREGISTRY_H

#include <dinrail/Device.h>

#include <pybind11/pybind11.h>

namespace dinrail::bindings::core
{

/**
 * @brief Create the Python callable that retrieves @p Interface from a dinrail.Device.
 *
 * The callable takes a dinrail.Device and returns a non-owning reference to the interface,
 * or None if the device does not provide it. The returned interface keeps the Python device
 * alive, but it is invalidated when the device is closed or opened again.
 *
 * The callable instantiates dinrail::Device::view<Interface>() in the module that calls this
 * function, so dinrail itself never needs to know @p Interface.
 */
template <class Interface> pybind11::cpp_function makeInterfaceViewer()
{
    namespace py = ::pybind11;
    return py::cpp_function(
        [](::dinrail::Device& device) -> Interface* {
            Interface* result = nullptr;
            device.view(result);
            return result;
        },
        py::arg("device"),
        py::return_value_policy::reference,
        py::keep_alive<0, 1>(),
        py::call_guard<py::gil_scoped_release>());
}

/**
 * @brief Register the Python class of a dinrail interface, so that it can be retrieved with
 * dinrail.Device.view().
 *
 * @param dinrailModule The `dinrail.bindings` module.
 * @param interfaceClass The pybind11 class binding the interface.
 */
template <class Interface, class... Options>
void registerInterface(const pybind11::module_& dinrailModule,
                       const pybind11::class_<Interface, Options...>& interfaceClass)
{
    dinrailModule.attr("registerInterface")(interfaceClass, makeInterfaceViewer<Interface>());
}

/**
 * @brief Register the Python class of an interface, so that it can be retrieved with
 * dinrail.Device.view().
 *
 * This is meant to be used by the Python bindings of interfaces that are not defined in
 * dinrail, for example interfaces provided by a device plugin:
 *
 * ~~~cpp
 * #include <dinrail/bindings/core/InterfaceRegistry.h>
 *
 * PYBIND11_MODULE(my_plugin_bindings, m)
 * {
 *     pybind11::class_<IMyInterface> myInterface(m, "IMyInterface");
 *     myInterface.def("doSomething", &IMyInterface::doSomething);
 *     dinrail::bindings::core::registerInterface(myInterface);
 * }
 * ~~~
 *
 * Then, in Python:
 *
 * ~~~python
 * import dinrail
 * import my_plugin_bindings
 *
 * device = dinrail.Device()
 * device.open({"device": "my_device"})
 * my_interface = device.view(my_plugin_bindings.IMyInterface)
 * ~~~
 *
 * The module calling this function must link dinrail::dinrail, and must be built with a
 * pybind11 version that is ABI-compatible with the one used to build the dinrail bindings,
 * so that the dinrail.Device type is shared between the two modules.
 *
 * @param interfaceClass The pybind11 class binding the interface.
 */
template <class Interface, class... Options>
void registerInterface(const pybind11::class_<Interface, Options...>& interfaceClass)
{
    registerInterface(pybind11::module_::import("dinrail.bindings"), interfaceClass);
}

} // namespace dinrail::bindings::core

#endif // DINRAIL_BINDINGS_CORE_INTERFACEREGISTRY_H
