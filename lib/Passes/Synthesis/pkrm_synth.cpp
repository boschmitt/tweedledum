/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Operators/Reversible.h"
#include "tweedledum/Passes/Synthesis/pkrm_synth.h"

#include <cassert>

namespace tweedledum {

// Synthesize a quantum circuit from a function by computing PKRM representation
// PKRM: Pseudo-Kronecker Read-Muller expression---a special case of an ESOP form.
void pkrm_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    kitty::dynamic_truth_table const& function)
{
    uint32_t const num_controls = function.num_vars();
    assert(qubits.size() >= (num_controls + 1u));

    WireRef const target = qubits.back();
    std::vector<WireRef> wires;
    wires.reserve(num_controls);
    for (auto const& cube : kitty::esop_from_optimum_pkrm(function)) {
        auto bits = cube._bits;
        auto mask = cube._mask;
        for (uint32_t v = 0u; mask; mask >>= 1, bits >>= 1, ++v) {
            if ((mask & 1) == 0u) {
                continue;
            }
            wires.push_back((bits & 1) ? qubits.at(v) : !qubits.at(v));
        }
        wires.push_back(target);
        circuit.apply_operator(Op::X(), wires);
        wires.clear();
    }
}

Circuit pkrm_synth(kitty::dynamic_truth_table const& function)
{
    Circuit circuit;
    // Create the necessary qubits
    std::vector<WireRef> wires;
    wires.reserve(function.num_vars());
    for (uint32_t i = 0u; i < function.num_vars() + 1; ++i) {
        wires.emplace_back(circuit.create_qubit());
    }
    pkrm_synth(circuit, wires, function);
    return circuit;
}

} // namespace tweedledum
