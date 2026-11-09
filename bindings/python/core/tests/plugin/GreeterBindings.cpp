// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include "IGreeter.h"

#include <dinrail/bindings/core/InterfaceRegistry.h>

#include <pybind11/pybind11.h>

// Bindings of an interface defined outside of dinrail, as a device plugin would provide them.
PYBIND11_MODULE(dinrail_test_greeter, m)
{
    namespace py = ::pybind11;

    py::class_<IGreeter> greeter(m, "IGreeter");
    greeter.def("greet", &IGreeter::greet, py::arg("name"));
    dinrail::bindings::core::registerInterface(greeter);
}
