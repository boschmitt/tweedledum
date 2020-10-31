/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Operators/Reversible.h"
#include "tweedledum/Passes/Synthesis/pprm_synth.h"

#include <cassert>

namespace tweedledum {

// Synthesize a quantum circuit from a function by computing PPRM representation
// 
// PPRM: The positive polarity Reed-Muller form is an ESOP, where each variable has
// positive polarity (not complemented form). PPRM is a canonical expression, so further
// minimization is not possible.
void pprm_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    kitty::dynamic_truth_table const& function)
{
    uint32_t const num_controls = function.num_vars();
    assert(qubits.size() >= (num_controls + 1u));

    WireRef const target = qubits.back();
    std::vector<WireRef> wires;
    wires.reserve(num_controls + 1);
    for (auto const& cube : kitty::esop_from_pprm(function)) {
        auto bits = cube._bits;
        for (uint32_t v = 0u; bits; bits >>= 1, ++v) {
            if ((bits & 1) == 0u) {
                continue;
            }
            wires.push_back(qubits.at(v));
        }
        wires.push_back(target);
        circuit.apply_operator(Op::X(), wires);
        wires.clear();
    }
}

Circuit pprm_synth(kitty::dynamic_truth_table const& function)
{
    Circuit circuit;
    // Create the necessary qubits
    std::vector<WireRef> wires;
    wires.reserve(function.num_vars());
    for (uint32_t i = 0u; i < function.num_vars() + 1; ++i) {
        wires.emplace_back(circuit.create_qubit());
    }
    pprm_synth(circuit, wires, function);
    return circuit;
}

} // namespace tweedledum
