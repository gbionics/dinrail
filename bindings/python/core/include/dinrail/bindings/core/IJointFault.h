// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_BINDINGS_CORE_IJOINTFAULT_H
#define DINRAIL_BINDINGS_CORE_IJOINTFAULT_H

#include <pybind11/pybind11.h>

namespace dinrail::bindings::core
{

void CreateIJointFault(pybind11::module_& module);

} // namespace dinrail::bindings::core

#endif // DINRAIL_BINDINGS_CORE_IJOINTFAULT_H
