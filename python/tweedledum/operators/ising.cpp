/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "operators.h"

void init_ising_operators(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Op::Rxx>(module, "Rxx")
        .def(py::init<double const>());

    py::class_<Op::Ryy>(module, "Ryy")
        .def(py::init<double const>());

    py::class_<Op::Rzz>(module, "Rzz")
        .def(py::init<double const>());
}
