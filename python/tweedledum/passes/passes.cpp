/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "../nlohmann_json.h"

#include <pybind11/stl.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Passes/Analysis.h>
#include <tweedledum/Passes/Decomposition.h>
#include <tweedledum/Passes/Optimization.h>
#include <tweedledum/Passes/Mapping.h>
#include <tweedledum/Passes/Utility.h>

void init_Passes(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    // Analysis
    module.def("compute_alap_layers", &compute_alap_layers, 
        "Compute instructions' ALAP layer.");

    module.def("compute_asap_layers", &compute_asap_layers, 
        "Compute instructions' ASAP layer.");

    module.def("compute_critical_paths", &compute_critical_paths, 
        "Compute circuit critical path(s).");

    module.def("compute_depth", &compute_depth, "Compute circuit depth.");

    module.def("count_operators", &count_operators, "Operators couting pass.");


    // Decomposition
    module.def("barenco_decomp", 
        py::overload_cast<Circuit const&, nlohmann::json const&>(&barenco_decomp),
        py::arg("circuit"), py::arg("config") = nlohmann::json(),
        "Barrenco decomposition pass.");

    module.def("bridge_decomp",
        py::overload_cast<Device const&, Circuit const&>(&bridge_decomp),
        "Bridge operators decomposition pass.");

    module.def("parity_decomp",
        py::overload_cast<Circuit const&>(&parity_decomp),
        "Parity operators decomposition pass.");

    // Mapping
    module.def("bridge_map", &bridge_map);

    module.def("jit_map", &jit_map);

    module.def("sabre_map", &sabre_map);

    // Optimization
    module.def("gate_cancellation", &gate_cancellation, "Gate cancellation optimization.");

    module.def("linear_resynth", &linear_resynth, 
        py::arg("original"), py::arg("config") = nlohmann::json(),
        "Resynthesize linear parts of the quantum circuit.");

    module.def("phase_folding", &phase_folding, "Phase folding optimization.");

    // Utility
    module.def("inverse", &inverse,
        "Invert (take adjoint of) a circuit.");

    module.def("reverse", &reverse,
        "Reverse a circuit.");

    module.def("shallow_duplicate", &shallow_duplicate,
        "Creates a new circuit with same wires as the original.");
}
