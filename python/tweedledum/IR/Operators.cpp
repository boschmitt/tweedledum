/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "Operators.h"

void init_Operators(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    // Reversible
    py::class_<Op::X>(module, "X")
        .def(py::init<>());

    // Clifford+T
    py::class_<Op::H>(module, "H")
        .def(py::init<>());

    py::class_<Op::S>(module, "S")
        .def(py::init<>());

    py::class_<Op::T>(module, "T")
        .def(py::init<>());

    py::class_<Op::Tdg>(module, "Tdg")
        .def(py::init<>());

    py::class_<Op::Z>(module, "Z")
        .def(py::init<>());
}
