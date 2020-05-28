/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <fmt/format.h>
#include <kitty/dynamic_truth_table.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <pybind11/pybind11.h>

namespace mockturtle {

std::string xag_simulate(xag_network ntk)
{
	auto const results = simulate<kitty::dynamic_truth_table>(
	    ntk, default_simulator<kitty::dynamic_truth_table>(ntk.num_pis()));

	return kitty::to_binary(results.at(0));
}

} // namespace mockturtle

void init_mockturtle(pybind11::module& m)
{
	using namespace mockturtle;
	namespace py = pybind11;

	m.def("simulate", &xag_simulate, "A function simulate a XAG");

	py::class_<xag_network::signal>(m, "xag_signal");

	py::class_<xag_network>(m, "xag_network")
	    .def(py::init<>())
	    .def("get_constant", &xag_network::get_constant)
	    .def("create_pi", &xag_network::create_pi, py::arg("name") = "")
	    .def("create_po", &xag_network::create_po, py::arg("f"),
	        py::arg("name") = "")
	    // Operations
	    .def("create_not", &xag_network::create_not)
	    .def("create_and", &xag_network::create_and)
	    .def("create_nand", &xag_network::create_nand)
	    .def("create_or", &xag_network::create_or)
	    .def("create_nor", &xag_network::create_nor)
	    .def("create_xor", &xag_network::create_xor)
	    .def("create_xnor", &xag_network::create_xnor)
	    // Structural properties
	    .def("num_gates", &xag_network::num_gates)
	    .def("num_pis", &xag_network::num_pis)
	    .def("num_pos", &xag_network::num_pos);
}

