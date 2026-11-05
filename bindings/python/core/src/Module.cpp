// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/bindings/core/Device.h>
#include <dinrail/bindings/core/IBattery.h>
#include <dinrail/bindings/core/Module.h>
#include <dinrail/bindings/core/Parameters.h>
#include <dinrail/bindings/core/RuntimeContext.h>
#include <dinrail/bindings/core/Status.h>

namespace dinrail::bindings::core
{

void CreateModule(pybind11::module_& module)
{
    CreateStatus(module);
    CreateParameters(module);
    CreateRuntimeContext(module);
    CreateDevice(module);

    // Interfaces are registered through the same mechanism available to external bindings.
    CreateIBattery(module);
}

} // namespace dinrail::bindings::core
