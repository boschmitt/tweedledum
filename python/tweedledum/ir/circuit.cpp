/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "circuit.h"

#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <tweedledum/Parser/qasm.h>
#include <tweedledum/Parser/tfc.h>
#include <tweedledum/Operators/All.h>
#include <tweedledum/Utils/Visualization/string_utf8.h>

void init_Circuit(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Circuit>(module, "Circuit")
        .def_static("from_qasm_file", &qasm::parse_source_file)
        .def_static("from_qasm_string", &qasm::parse_source_buffer)
        .def_static("from_tfc_file", &tfc::parse_source_file)
        .def(py::init<>())
        // Properties
        .def("num_ancillae", &Circuit::num_ancillae)
        .def("num_cbits", &Circuit::num_cbits)
        .def("num_qubits", &Circuit::num_qubits)
        .def("num_wires", &Circuit::num_wires)
        // Wires
        .def("create_cbit", py::overload_cast<>(&Circuit::create_cbit))
        .def("create_qubit", py::overload_cast<>(&Circuit::create_qubit))
        .def("request_ancilla", &Circuit::request_ancilla)
        .def("release_ancilla", &Circuit::release_ancilla)
        // Extension Operators
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::TruthTable const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Unitary const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        // Ising Operators
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Rxx const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Ryy const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Rzz const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        // Meta Operators
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Barrier const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        // Standard Operators
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::H const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Measure const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::P const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Rx const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Ry const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Rz const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::S const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Sdg const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Swap const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::T const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Tdg const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::X const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Y const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Op::Z const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Instruction const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("apply_operator", static_cast<InstRef (Circuit::*)(Instruction const&)>(&Circuit::apply_operator))
        .def("apply_operator", static_cast<InstRef (Circuit::*)(pybind11::object const&, std::vector<Qubit> const&, std::vector<Cbit> const&)>(&Circuit::apply_operator),
             py::arg("optor"), py::arg("qubits"), py::arg("cbits") = std::vector<Cbit>())
        .def("append", &Circuit::append)
        // Python stuff
        .def("__iter__", [](Circuit const& c) { return py::make_iterator(c.py_begin(), c.py_end()); }, py::keep_alive<0, 1>())
        .def("__len__", &Circuit::size)
        .def("__str__", [](Circuit const& c) { return to_string_utf8(c); });

}
