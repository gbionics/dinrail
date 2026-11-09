// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/bindings/core/Module.h>

// Create the Python module
PYBIND11_MODULE(bindings, m)
{
    m.doc() = "dinrail bindings";

    dinrail::bindings::core::CreateModule(m);
}
