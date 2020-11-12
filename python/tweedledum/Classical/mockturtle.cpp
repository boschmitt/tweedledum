/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <mockturtle/networks/xag.hpp>
#include <pybind11/pybind11.h>

void init_mockturtle(pybind11::module& module)
{
    using namespace mockturtle;
    namespace py = pybind11;

    using signal = xag_network::signal;

    py::class_<signal>(module, "Signal")
        .def("__not__", [](signal const& lhs) { return !lhs; })
        .def("__eq__", [](signal const& lhs, signal *rhs) { return rhs && lhs == *rhs; })
        .def("__ne__", [](signal const& lhs, signal *rhs) { return !rhs || lhs != *rhs; });

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
        // Structural properties
        .def("num_gates", &xag_network::num_gates)
        .def("num_pis", &xag_network::num_pis)
        .def("num_pos", &xag_network::num_pos);

}
