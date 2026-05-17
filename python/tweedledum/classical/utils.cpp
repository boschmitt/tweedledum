/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <tweedledum/Utils/Classical/xag_optimize.h>
#include <tweedledum/Utils/Classical/xag_simulate.h>
#include <tweedledum/Utils/Classical/xag_stats.h>

void init_classical_utils(nanobind::module_& module)
{
    namespace nb = nanobind;
    using namespace tweedledum;

    module.def("optimize", &xag_optimize, "A function optimize a XAG");

    module.def("simulate", static_cast<std::vector<kitty::dynamic_truth_table> (*)(mockturtle::xag_network const&)>(&xag_simulate), "A function simulate a XAG");
    module.def("simulate", static_cast<std::vector<bool> (*)(mockturtle::xag_network const&, std::vector<bool> const&)>(&xag_simulate), "A function simulate a XAG");
}
