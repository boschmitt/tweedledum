/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "passes.h"

#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Passes/Analysis/depth.h>
#include <tweedledum/Passes/Decomposition.h>
#include <tweedledum/Passes/Synthesis.h>

void init_Passes(pybind11::module& module)
{
    using namespace tweedledum;
    namespace py = pybind11;

    // Analysis
    module.def("depth", &depth, "Compute depth pass.");

    // Decomposition
    module.def("barenco_decomp", py::overload_cast<Circuit const&, nlohmann::json const&>(&barenco_decomp),
    py::arg("circuit"), py::arg("config") = nlohmann::json(),
    "Barrenco decomposition depth pass.");

    // Synthesis 
    module.def("lhrs_synth",  py::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&lhrs_synth),
    py::arg("xag"), py::arg("config") = nlohmann::json(),
    "LUT-based Hierarchical Synthesis from a XAG representation.");

    module.def("lhrs_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, mockturtle::xag_network const&, nlohmann::json const&>(&lhrs_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("xag"), py::arg("config") = nlohmann::json(),
    "LUT-based Hierarchical Synthesis inplace from a XAG representation.");

    module.def("pkrm_synth", py::overload_cast<kitty::dynamic_truth_table const&, nlohmann::json const&>(&pkrm_synth),
    py::arg("function"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit from a function by computing PKRM representation.");

    module.def("pkrm_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, kitty::dynamic_truth_table const&, nlohmann::json const&>(&pkrm_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("function"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit inplace from a function by computing PKRM representation.");

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

    module.def("xag_synth",  py::overload_cast<mockturtle::xag_network const&, nlohmann::json const&>(&xag_synth),
    py::arg("xag"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit from a XAG representation.");

    module.def("xag_synth",  py::overload_cast<Circuit&, std::vector<WireRef> const&, mockturtle::xag_network const&, nlohmann::json const&>(&xag_synth),
    py::arg("circuit"), py::arg("qubits"), py::arg("xag"), py::arg("config") = nlohmann::json(),
    "Synthesize a quantum circuit inplace from a XAG representation.");
}
