/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "operators.h"

void init_std_operators(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Op::H>(module, "H")
        .def(py::init<>());

    py::class_<Op::Measure>(module, "Measure")
        .def(py::init<>());

    py::class_<Op::P>(module, "P")
        .def(py::init<double const>());

    py::class_<Op::Rx>(module, "Rx")
        .def(py::init<double const>());

    py::class_<Op::Ry>(module, "Ry")
        .def(py::init<double const>());

    py::class_<Op::Rz>(module, "Rz")
        .def(py::init<double const>());

    py::class_<Op::S>(module, "S")
        .def(py::init<>());

    py::class_<Op::Sdg>(module, "Sdg")
        .def(py::init<>());

    py::class_<Op::Swap>(module, "Swap")
        .def(py::init<>());

    py::class_<Op::T>(module, "T")
        .def(py::init<>());

    py::class_<Op::Tdg>(module, "Tdg")
        .def(py::init<>());

    py::class_<Op::X>(module, "X")
        .def(py::init<>());

    py::class_<Op::Y>(module, "Y")
        .def(py::init<>());

    py::class_<Op::Z>(module, "Z")
        .def(py::init<>());
}
