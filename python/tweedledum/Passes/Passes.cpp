/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include <pybind11/pybind11.h>
#include <tweedledum/IR/Circuit.h>
#include <tweedledum/Passes/Analysis/depth.h>
// #include <tweedledum/Passes/Optimization/x_cancellation.h>
#include <tweedledum/Passes/Synthesis/pkrm_synth.h>
#include <tweedledum/Passes/Synthesis/pprm_synth.h>

PYBIND11_MODULE(libTweedledumPasses, module)
{
    using namespace tweedledum;
    namespace py = pybind11;
    module.doc() = "tweedledum Passes";

    // Analysis
    module.def("depth", &depth, "Compute depth pass.");

    // Synthesis 
    module.def("pkrm_synth", (Circuit (*)(kitty::dynamic_truth_table const&)) &pkrm_synth, 
    "Synthesize a quantum circuit from a function by computing PKRM representation.");

    module.def("pkrm_synth", (void (*)(Circuit&, std::vector<WireRef> const&, kitty::dynamic_truth_table const&)) &pkrm_synth,
    "Synthesize a quantum circuit from a function by computing PKRM representation.");

    module.def("pprm_synth", (Circuit (*)(kitty::dynamic_truth_table const&)) &pprm_synth, 
    "Synthesize a quantum circuit from a function by computing PPRM representation.");

    module.def("pprm_synth", (void (*)(Circuit&, std::vector<WireRef> const&, kitty::dynamic_truth_table const&)) &pprm_synth,
    "Synthesize a quantum circuit from a function by computing PPRM representation.");
}
