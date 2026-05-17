/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "operators.h"

void init_std_operators(nanobind::module_& module)
{
    using namespace tweedledum;
    namespace nb = nanobind;

    nb::class_<Op::H>(module, "H")
        .def(nb::init<>());

    nb::class_<Op::Measure>(module, "Measure")
        .def(nb::init<>());

    nb::class_<Op::P>(module, "P")
        .def(nb::init<double const>());

    nb::class_<Op::Rx>(module, "Rx")
        .def(nb::init<double const>());

    nb::class_<Op::Ry>(module, "Ry")
        .def(nb::init<double const>());

    nb::class_<Op::Rz>(module, "Rz")
        .def(nb::init<double const>());

    nb::class_<Op::S>(module, "S")
        .def(nb::init<>());

    nb::class_<Op::Sdg>(module, "Sdg")
        .def(nb::init<>());

    nb::class_<Op::Sx>(module, "Sx")
        .def(nb::init<>());

    nb::class_<Op::Sxdg>(module, "Sxdg")
        .def(nb::init<>());

    nb::class_<Op::Swap>(module, "Swap")
        .def(nb::init<>());

    nb::class_<Op::T>(module, "T")
        .def(nb::init<>());

    nb::class_<Op::Tdg>(module, "Tdg")
        .def(nb::init<>());

    nb::class_<Op::X>(module, "X")
        .def(nb::init<>());

    nb::class_<Op::Y>(module, "Y")
        .def(nb::init<>());

    nb::class_<Op::Z>(module, "Z")
        .def(nb::init<>());
}
