/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "../nlohmann_json.h"

#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Passes/Analysis.h>
#include <tweedledum/Passes/Decomposition.h>
#include <tweedledum/Passes/Optimization.h>
#include <tweedledum/Passes/Mapping.h>
#include <tweedledum/Passes/Utility.h>

void init_Passes(nanobind::module_& module)
{
    using namespace tweedledum;
    namespace nb = nanobind;

    // Analysis
    module.def("compute_alap_layers", &compute_alap_layers,
        "Compute instructions' ALAP layer.");

    module.def("compute_asap_layers", &compute_asap_layers,
        "Compute instructions' ASAP layer.");

    module.def("compute_critical_paths", &compute_critical_paths,
        "Compute circuit critical path(s).");

    module.def("compute_cuts", &compute_cuts, "Compute circuit cuts.");

    module.def("compute_depth", &compute_depth, "Compute circuit depth.");

    module.def("count_operators", &count_operators, "Operators couting pass.");


    // Decomposition
    module.def("barenco_decomp",
        nb::overload_cast<Circuit const&, nlohmann::json const&>(&barenco_decomp),
        nb::arg("circuit"), nb::arg("config") = nlohmann::json(),
        "Barrenco decomposition pass.");

    module.def("bridge_decomp",
        nb::overload_cast<Device const&, Circuit const&, nlohmann::json const&>(&bridge_decomp),
        nb::arg("device"), nb::arg("circuit"), nb::arg("config") = nlohmann::json(),
        "Bridge operators decomposition pass.");

    module.def("parity_decomp",
        nb::overload_cast<Circuit const&, nlohmann::json const&>(&parity_decomp),
        nb::arg("circuit"), nb::arg("config") = nlohmann::json(),
        "Parity operators decomposition pass.");

    // Mapping
    module.def("bridge_map", &bridge_map);

    module.def("jit_map", &jit_map);

    module.def("sabre_map", &sabre_map);

    // Optimization
    module.def("gate_cancellation", &gate_cancellation, "Gate cancellation optimization.");

    module.def("linear_resynth", &linear_resynth,
        nb::arg("original"), nb::arg("config") = nlohmann::json(),
        "Resynthesize linear parts of the quantum circuit.");

    module.def("phase_folding", &phase_folding, "Phase folding optimization.");

    module.def("steiner_resynth", &steiner_resynth,
        nb::arg("device"), nb::arg("original"), nb::arg("config") = nlohmann::json(),
        "Coupling-aware resynthesize linear parts of the quantum circuit.");

    // Utility
    module.def("inverse", &inverse,
        "Invert (take adjoint of) a circuit.");

    module.def("reverse", &reverse,
        "Reverse a circuit.");

    module.def("shallow_duplicate", &shallow_duplicate,
        "Creates a new circuit with same wires as the original.");
}
