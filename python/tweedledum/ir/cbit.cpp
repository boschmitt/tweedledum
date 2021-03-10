/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <tweedledum/IR/Cbit.h>

void init_Cbit(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Cbit> cbit(module, "Cbit");
    cbit.def("polarity", &Cbit::polarity)
        .def("uid", &Cbit::uid)
        .def("__index__", [](Cbit const& bit) { return static_cast<uint32_t>(bit); })
        .def("__invert__", [](Cbit const& lhs) { return !lhs; })
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::enum_<Cbit::Polarity>(cbit, "Polarity")
        .value("negative", Cbit::Polarity::negative)
        .value("positive", Cbit::Polarity::positive)
        .export_values();
}
