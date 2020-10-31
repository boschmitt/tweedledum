/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "Instruction.h"
#include "Operators.h"

#include <pybind11/stl.h>
#include <tweedledum/IR/Instruction.h>

void init_Instruction(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<InstRef>(module, "InstRef")
        .def("uid", &InstRef::uid);

    py::class_<Instruction>(module, "Instruction")
        .def("kind", &Instruction::kind)
        .def("num_wires", &Instruction::num_wires)
        .def("wires", &Instruction::wires)
        .def("py_op", [](Instruction const& i) {
            auto const& temp = i.cast<python::PyOperator>();
            return temp.obj();
        });
}
