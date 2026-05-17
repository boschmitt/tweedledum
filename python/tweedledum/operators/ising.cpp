/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "operators.h"

void init_ising_operators(nanobind::module_& module)
{
    using namespace tweedledum;
    namespace nb = nanobind;

    nb::class_<Op::Rxx>(module, "Rxx")
        .def(nb::init<double const>());

    nb::class_<Op::Ryy>(module, "Ryy")
        .def(nb::init<double const>());

    nb::class_<Op::Rzz>(module, "Rzz")
        .def(nb::init<double const>());
}
