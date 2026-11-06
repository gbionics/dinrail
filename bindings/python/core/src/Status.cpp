// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <pybind11/pybind11.h>

#include <dinrail/Status.h>
#include <dinrail/bindings/core/Status.h>

#include <string>

namespace dinrail::bindings::core
{

void CreateStatus(pybind11::module_& module)
{
    namespace py = ::pybind11;
    using ::dinrail::Status;
    using ::dinrail::StatusCode;

    py::enum_<StatusCode>(module, "StatusCode")
        .value("Ok", StatusCode::Ok)
        .value("Cancelled", StatusCode::Cancelled)
        .value("Unknown", StatusCode::Unknown)
        .value("InvalidArgument", StatusCode::InvalidArgument)
        .value("DeadlineExceeded", StatusCode::DeadlineExceeded)
        .value("NotFound", StatusCode::NotFound)
        .value("AlreadyExists", StatusCode::AlreadyExists)
        .value("PermissionDenied", StatusCode::PermissionDenied)
        .value("ResourceExhausted", StatusCode::ResourceExhausted)
        .value("FailedPrecondition", StatusCode::FailedPrecondition)
        .value("Aborted", StatusCode::Aborted)
        .value("OutOfRange", StatusCode::OutOfRange)
        .value("Unimplemented", StatusCode::Unimplemented)
        .value("Internal", StatusCode::Internal)
        .value("Unavailable", StatusCode::Unavailable)
        .value("DataLoss", StatusCode::DataLoss)
        .value("Unauthenticated", StatusCode::Unauthenticated);

    py::class_<Status>(module, "Status")
        .def(py::init<>())
        .def(py::init<StatusCode>(), py::arg("code"))
        .def("ok", [](const Status& self) { return self.ok(); })
        .def("code", [](const Status& self) { return self.code(); })
        .def("ToString", &Status::ToString)
        .def("__str__", &Status::ToString)
        .def("__repr__",
             [](const Status& self) { return "Status(StatusCode." + self.ToString() + ")"; })
        .def(
            "__eq__",
            [](const Status& self, const Status& other) { return self == other; },
            py::is_operator());
}

} // namespace dinrail::bindings::core
