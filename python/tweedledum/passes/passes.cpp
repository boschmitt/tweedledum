/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "passes.h"

#include <pybind11/eigen.h>
#include <pybind11/stl.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Passes/Analysis/depth.h>
#include <tweedledum/Passes/Optimization.h>
#include <tweedledum/Passes/Decomposition.h>
#include <tweedledum/Passes/Synthesis.h>
#include <tweedledum/Passes/Utility/shallow_duplicate.h>

void init_Passes(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    // Analysis
    module.def("depth", &depth, "Compute depth pass.");

    // Decomposition
    module.def("barenco_decomp", py::overload_cast<Circuit const&, nlohmann::json const&>(&barenco_decomp),
    py::arg("circuit"), py::arg("config") = nlohmann::json(),
    "Barrenco decomposition pass.");

    module.def("parity_decomp", py::overload_cast<Circuit const&>(&parity_decomp),
    "Parity operators decomposition pass.");

    // Optimization
    module.def("linear_resynth", &linear_resynth, py::arg("original"), py::arg("config") = nlohmann::json(),
    "Resynthesize linear parts of the quantum circuit.");

    module.def("phase_folding", &phase_folding, "Phase folding optimization.");

    // Synthesis 
    module.def("decomp_synth",  py::overload_cast<std::vector<uint32_t> const&>(&decomp_synth),
    "Reversible synthesis based on functional decomposition.");

    module.def("decomp_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&,
    std::vector<uint32_t> const&>(&decomp_synth),
    "Reversible synthesis based on functional decomposition.");

    module.def("lhrs_synth",  py::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&lhrs_synth),
    py::arg("xag"), py::arg("config") = nlohmann::json(),
    "LUT-based Hierarchical Synthesis from a XAG representation.");

    module.def("lhrs_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, mockturtle::xag_network const&, nlohmann::json const&>(&lhrs_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("xag"), py::arg("config") = nlohmann::json(),
    "LUT-based Hierarchical Synthesis inplace from a XAG representation.");

    module.def("linear_synth",  py::overload_cast<BMatrix const&, nlohmann::json const&>(&linear_synth),
    py::arg("matrix"), py::arg("config") = nlohmann::json(),
    "Synthesis of linear reversible circuits (CNOT synthesis).");

    module.def("linear_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, BMatrix const&, nlohmann::json const&>(&linear_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("matrix"), py::arg("config") = nlohmann::json(),
    "Reversible synthesis based on functional decomposition.");

    module.def("pkrm_synth", py::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&pkrm_synth),
    py::arg("function"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit from a function by computing PKRM representation.");

    module.def("pkrm_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&pkrm_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("function"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit inplace from a function by computing PKRM representation.");

    module.def("pkrm_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, Instruction const&, nlohmann::json const&>(&pkrm_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("inst"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit inplace from a Instruction by computing PKRM representation.");

    module.def("pkrm_synth",  py::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&pkrm_synth),
    py::arg("xag"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit from a XAG by computing PKRM representation.");

    module.def("pkrm_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, mockturtle::xag_network const&, nlohmann::json const&>(&pkrm_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("xag"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit inplace from a XAG by computing PKRM representation.");
    
    module.def("pprm_synth",  py::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&pprm_synth),
    py::arg("function"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit from a function by computing PPRM representation.");

    module.def("pprm_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&pprm_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("function"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit inplace from a function by computing PPRM representation.");

    module.def("spectrum_synth",  py::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&spectrum_synth),
    py::arg("function"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit from a function by computing the Rademacher-Walsh spectrum.");

    module.def("spectrum_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&spectrum_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("function"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit inplace from a function by computing the Rademacher-Walsh spectrum.");

    module.def("transform_synth",  py::overload_cast<std::vector<uint32_t> const&>(&transform_synth),
    "Reversible synthesis based on symbolic transformation.");

    module.def("transform_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&,
    std::vector<uint32_t> const&>(&transform_synth),
    "Reversible synthesis based on symbolic transformation.");

    module.def("xag_synth",  py::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&xag_synth),
    py::arg("xag"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit from a XAG representation.");

    module.def("xag_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, mockturtle::xag_network const&, nlohmann::json const&>(&xag_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("xag"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit inplace from a XAG representation.");

    // Utility
    module.def("shallow_duplicate", &shallow_duplicate,
    "Creates a new circuit with same wires as the original.");

}
