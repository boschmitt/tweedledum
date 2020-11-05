/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "Operators.h"

void init_ext_operators(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    // py::class_<Op::TruthTable>(module, "TruthTable")
    //     .def(py::init<>());

    // py::class_<Op::Unitary>(module, "Unitary")
    //     .def(py::init<>());
}
