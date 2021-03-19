/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <tweedledum/Utils/Classical/xag_optimize.h>
#include <tweedledum/Utils/Classical/xag_simulate.h>
#include <tweedledum/Utils/Classical/xag_stats.h>

void init_classical_utils(pybind11::module& module)
{
    namespace py = pybind11;
    using namespace tweedledum;

    module.def("optimize", &xag_optimize, "A function optimize a XAG");

    module.def("simulate", static_cast<std::vector<kitty::dynamic_truth_table> (*)(mockturtle::xag_network const&)>(&xag_simulate), "A function simulate a XAG");
    module.def("simulate", static_cast<std::vector<bool> (*)(mockturtle::xag_network const&, std::vector<bool> const&)>(&xag_simulate), "A function simulate a XAG");
}
