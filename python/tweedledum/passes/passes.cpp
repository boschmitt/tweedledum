/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "../nlohmann_json.h"

#include <pybind11/stl.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Passes/Analysis/depth.h>
#include <tweedledum/Passes/Decomposition.h>
#include <tweedledum/Passes/Optimization.h>
#include <tweedledum/Passes/Mapping.h>
#include <tweedledum/Passes/Utility/shallow_duplicate.h>

void init_Passes(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    // Analysis
    module.def("depth", &depth, "Compute depth pass.");

    // Decomposition
    module.def("barenco_decomp", 
        py::overload_cast<Circuit const&, nlohmann::json const&>(&barenco_decomp),
        py::arg("circuit"), py::arg("config") = nlohmann::json(),
        "Barrenco decomposition pass.");

    module.def("parity_decomp",
        py::overload_cast<Circuit const&>(&parity_decomp),
        "Parity operators decomposition pass.");

    // Mapping
    module.def("jit_map", &jit_map);

    module.def("sabre_map", &sabre_map);

    // Optimization
    module.def("linear_resynth", &linear_resynth, 
        py::arg("original"), py::arg("config") = nlohmann::json(),
        "Resynthesize linear parts of the quantum circuit.");

    module.def("phase_folding", &phase_folding, "Phase folding optimization.");

    // Utility
    module.def("shallow_duplicate", &shallow_duplicate,
        "Creates a new circuit with same wires as the original.");
}
