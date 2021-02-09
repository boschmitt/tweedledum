/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <mockturtle/algorithms/exorcism.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/dimacs_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/xag.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

void init_mockturtle(pybind11::module& module)
{
    namespace py = pybind11;
    using namespace mockturtle;
    using signal = xag_network::signal;

    py::class_<signal>(module, "Signal")
        .def("__invert__", [](signal const& lhs) { return !lhs; })
        .def(py::self == py::self)
        .def(py::self != py::self);

    py::class_<xag_network>(module, "LogicNetwork")
        .def(py::init<>())
        .def("get_constant", &xag_network::get_constant)
        .def("create_pi", &xag_network::create_pi, py::arg("name") = "")
        .def("create_po", &xag_network::create_po, py::arg("f"), py::arg("name") = "")
        // Operations
        .def("create_not", &xag_network::create_not)
        .def("create_and", &xag_network::create_and)
        .def("create_nand", &xag_network::create_nand)
        .def("create_or", &xag_network::create_or)
        .def("create_nor", &xag_network::create_nor)
        .def("create_xor", &xag_network::create_xor)
        .def("create_xnor", &xag_network::create_xnor)
        .def("create_nary_and", &xag_network::create_nary_and)
        .def("create_nary_or", &xag_network::create_nary_or)
        .def("create_nary_xor", &xag_network::create_nary_xor)
        // Structural properties
        .def("num_gates", &xag_network::num_gates)
        .def("num_pis", &xag_network::num_pis)
        .def("num_pos", &xag_network::num_pos);

    // IO
    module.def("read_aiger", [](std::string const filename) {
        xag_network xag;
        lorina::read_aiger(filename, aiger_reader(xag));
        return xag;
    }, "Create a LogicNetwork from a AIGER file.");

    module.def("read_dimacs", [](std::string const filename) {
        xag_network xag;
        lorina::read_dimacs(filename, dimacs_reader(xag));
        return xag;
    }, "Create a LogicNetwork from a DIMACS file.");

    module.def("read_verilog", [](std::string const filename) {
        xag_network xag;
        lorina::read_verilog(filename, verilog_reader(xag));
        return xag;
    }, "Create a LogicNetwork from a Verilog file.");

    module.def("write_verilog", [](xag_network const& xag, std::string const& filename) {
        write_verilog(xag, filename);
    }, "Write a LogicNetwork to a Verilog file.");

    // Optimization
    module.def("exorcism", py::overload_cast<std::vector<kitty::cube> const&, uint32_t>(&exorcism));

    module.def("exorcism", py::overload_cast<kitty::dynamic_truth_table const&>(&exorcism));
}
