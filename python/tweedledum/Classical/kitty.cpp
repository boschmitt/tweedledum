/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <kitty/kitty.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

void init_kitty(pybind11::module& module)
{
    using namespace kitty;
    namespace py = pybind11;

    py::class_<dynamic_truth_table>(module, "TruthTable")
        .def(py::init<uint32_t>())
        .def("num_vars", &dynamic_truth_table::num_vars)
        .def("__str__", [](dynamic_truth_table const& tt) { return to_binary(tt); });

    module.def("create_from_binary_string",  &create_from_binary_string<dynamic_truth_table>,
    "Constructs truth table from binary string.");
}
