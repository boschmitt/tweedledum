/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "Circuit.h"


#include <pybind11/stl.h>
#include <tweedledum/IR/Wire.h>
#include <tweedledum/Operators/Reversible.h>
#include <tweedledum/Operators/Standard.h>

void init_Circuit(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<WireRef>(module, "WireRef")
        .def("uid", &WireRef::uid);

    py::class_<Circuit>(module, "Circuit")
        .def(py::init<>())
        .def("create_qubit", py::overload_cast<>(&Circuit::create_qubit))
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::X const&, std::vector<WireRef> const&)>(&Circuit::apply_operator))
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::H const&, std::vector<WireRef> const&)>(&Circuit::apply_operator))
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::T const&, std::vector<WireRef> const&)>(&Circuit::apply_operator))
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Tdg const&, std::vector<WireRef> const&)>(&Circuit::apply_operator))
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Instruction const&, std::vector<WireRef> const&)>(&Circuit::apply_operator))
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Instruction const&)>(&Circuit::apply_operator))
        .def("apply_operator", static_cast<InstRef (Circuit::*)(pybind11::object const&, std::vector<WireRef> const&)>(&Circuit::apply_operator))
        .def("append", &Circuit::append)
        // Python stuff
        .def("__iter__", [](Circuit const& c) { return py::make_iterator(c.py_begin(), c.py_end()); }, py::keep_alive<0, 1>())
        .def("__len__", &Circuit::size);
}
