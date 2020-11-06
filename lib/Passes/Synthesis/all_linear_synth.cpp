/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Synthesis/all_linear_synth.h"
#include "tweedledum/Operators/Standard.h"

namespace tweedledum {

namespace {

// I added this level of indirection because I can implement this method with
// other codes or just using binary sequence.
inline void synthesize(Circuit& circuit, std::vector<WireRef> const& qubits, LinearPP parities)
{
    // Generate Gray code
    std::vector<uint32_t> gray_code(1u << circuit.num_qubits());
    for (uint32_t i = 0; i < (1u << circuit.num_qubits()); ++i) {
        gray_code.at(i) = ((i >> 1) ^ i);
    }

    // Initialize the parity of each qubit state
    // Applying phase gate to parities that consisting of just one variable
    // i is the index of the target
    std::vector<uint32_t> qubits_states(circuit.num_qubits(), 0);
    for (uint32_t i = 0u; i < circuit.num_qubits(); ++i) {
        qubits_states[i] = (1u << i);
        auto angle = parities.extract_term(qubits_states[i]);
        if (angle != 0.0) {
            circuit.apply_operator(Op::P(angle), {qubits[i]});
        }
    }

    for (uint32_t i = circuit.num_qubits() - 1; i > 0; --i) {
        for (uint32_t j = (1u << (i + 1)) - 1; j > (1u << i); --j) {
            uint32_t c0 = std::log2(gray_code[j] ^ gray_code[j - 1u]);
            circuit.apply_operator(Op::X(), {qubits[c0], qubits[i]});

            qubits_states[i] ^= qubits_states[c0];
            auto angle = parities.extract_term(qubits_states[i]);
            if (angle != 0.0) {
                circuit.apply_operator(Op::P(angle), {qubits[i]});
            }
        }
        uint32_t c1 = std::log2(gray_code[1 << i] ^ gray_code[(1u << (i + 1)) - 1u]);
        circuit.apply_operator(Op::X(), {qubits[c1], qubits[i]});

        qubits_states[i] ^= qubits_states[c1];
        auto angle = parities.extract_term(qubits_states[i]);
        if (angle != 0.0) {
            circuit.apply_operator(Op::P(angle), {qubits[i]});
        }
    }
}

}

void all_linear_synth(Circuit& circuit, std::vector<WireRef> const& qubits, LinearPP const& parities)
{
    if (parities.size() == 0) {
        return;
    }
    synthesize(circuit, qubits, parities);
}

Circuit all_linear_synth(uint32_t num_qubits, LinearPP const& parities)
{
    Circuit circuit;

    // Create the necessary qubits
    std::vector<WireRef> wires;
    wires.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        wires.emplace_back(circuit.create_qubit());
    }
    all_linear_synth(circuit, wires, parities);
    return circuit;
}

} // namespace tweedledum