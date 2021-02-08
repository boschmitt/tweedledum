/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "operators.h"

void init_meta_operators(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Op::Barrier>(module, "Barrier")
        .def(py::init<>());
}
