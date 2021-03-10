/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <tweedledum/IR/Qubit.h>

void init_Qubit(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Qubit> qubit(module, "Qubit");
    qubit.def("polarity", &Qubit::polarity)
        .def("uid", &Qubit::uid)
        .def("__index__", [](Qubit const& bit) { return static_cast<uint32_t>(bit); })
        .def("__invert__", [](Qubit const& lhs) { return !lhs; })
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::enum_<Qubit::Polarity>(qubit, "Polarity")
        .value("negative", Qubit::Polarity::negative)
        .value("positive", Qubit::Polarity::positive)
        .export_values();
}
