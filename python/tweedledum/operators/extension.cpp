/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "operators.h"

#include <kitty/kitty.hpp>
#include <mockturtle/networks/xag.hpp>
#include <nanobind/stl/vector.h>
#include <vector>

void init_ext_operators(nanobind::module_& module)
{
    using namespace tweedledum;
    namespace nb = nanobind;

    nb::class_<Op::LogicNetwork>(module, "LogicNetwork")
        .def(nb::init<mockturtle::xag_network>());

    nb::class_<Op::Permutation>(module, "Permutation")
        .def(nb::init<std::vector<uint32_t>>());

    nb::class_<Op::TruthTable>(module, "TruthTable")
        .def(nb::init<kitty::dynamic_truth_table, bool>(),
             nb::arg("tt"), nb::arg("is_phase") = false);

    // nb::class_<Op::Unitary>(module, "Unitary")
    //     .def(nb::init<>());
}
