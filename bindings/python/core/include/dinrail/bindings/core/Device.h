// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_BINDINGS_CORE_DEVICE_H
#define DINRAIL_BINDINGS_CORE_DEVICE_H

#include <pybind11/pybind11.h>

namespace dinrail::bindings::core
{

/**
 * @brief Bind dinrail::Device and the registry of the interfaces retrievable with Device.view().
 *
 * It must be called before registering any interface in @p module.
 */
void CreateDevice(pybind11::module_& module);

} // namespace dinrail::bindings::core

#endif // DINRAIL_BINDINGS_CORE_DEVICE_H
