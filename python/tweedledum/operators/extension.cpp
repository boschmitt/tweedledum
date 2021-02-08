/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "operators.h"

#include <kitty/kitty.hpp>
#include <mockturtle/networks/xag.hpp>
#include <vector>

void init_ext_operators(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    py::class_<Op::LogicNetwork>(module, "LogicNetwork")
        .def(py::init<mockturtle::xag_network>());

    py::class_<Op::Permutation>(module, "Permutation")
        .def(py::init<std::vector<uint32_t>>());

    py::class_<Op::TruthTable>(module, "TruthTable")
        .def(py::init<kitty::dynamic_truth_table, bool>(), 
             py::arg("tt"), py::arg("is_phase") = false);

    // py::class_<Op::Unitary>(module, "Unitary")
    //     .def(py::init<>());
}
