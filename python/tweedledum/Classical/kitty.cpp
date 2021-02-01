/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <kitty/kitty.hpp>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

void init_kitty(pybind11::module& module)
{
    using namespace kitty;
    namespace py = pybind11;

    py::class_<dynamic_truth_table>(module, "TruthTable")
        .def(py::init<uint32_t>())
        .def("num_vars", &dynamic_truth_table::num_vars)
        .def(~py::self)
        .def(py::self & py::self)
        .def(py::self | py::self)
        .def(py::self ^ py::self)
        .def(py::self == py::self)
        .def("__getitem__", [](dynamic_truth_table const& tt, size_t idx) -> bool {
            if (idx >= tt.num_bits()) {
                throw py::index_error();
            }
            return kitty::get_bit(tt, idx);
        })
        .def("__setitem__", [](dynamic_truth_table& tt, size_t idx, bool v) {
            if (idx >= tt.num_bits()) {
                throw py::index_error();
            }
            if (v) {
                kitty::set_bit(tt, idx);
            } else {
                kitty::clear_bit(tt, idx);
            }
        })
        .def("__str__", [](dynamic_truth_table const& tt) { return to_binary(tt); });

    // Constructors
    module.def("create_from_binary_string", &create_from_binary_string<dynamic_truth_table>,
    "Constructs truth table from binary string.");

    // Bit operations
    module.def("count_ones", &count_ones<dynamic_truth_table>,
    "Count ones in truth table.");

    module.def("get_minterms", &get_minterms<dynamic_truth_table>,
    "Computes all minterms in a truth table.");
}
