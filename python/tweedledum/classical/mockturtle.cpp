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
#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

void init_mockturtle(nanobind::module_& module)
{
    namespace nb = nanobind;
    using namespace mockturtle;
    using signal = xag_network::signal;

    nb::class_<signal>(module, "Signal")
        .def("__invert__", [](signal const& lhs) { return !lhs; })
        .def(nb::self == nb::self)
        .def(nb::self != nb::self);

    nb::class_<xag_network>(module, "LogicNetwork")
        .def(nb::init<>())
        .def("create_pi", &xag_network::create_pi, nb::arg("name") = "")
        .def("create_po", &xag_network::create_po, nb::arg("f"), nb::arg("name") = "")
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
        .def("num_pos", &xag_network::num_pos)
        // Node and signals
        .def("get_constant", [](xag_network const& xag, long value) {
            return xag.get_constant(static_cast<bool>(value));
        })
        .def("get_node", &xag_network::get_node)
        .def("pi_at", &xag_network::pi_at);

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
    module.def("exorcism", nb::overload_cast<std::vector<kitty::cube> const&, uint32_t>(&exorcism));

    module.def("exorcism", nb::overload_cast<kitty::dynamic_truth_table const&>(&exorcism));
}
