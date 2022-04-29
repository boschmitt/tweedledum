/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/all_linear_synth.h"
#include "tweedledum/Operators/Standard.h"

namespace tweedledum {

namespace {

// I added this level of indirection because I can implement this method with
// other codes or just using binary sequence.
inline void synthesize(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, LinPhasePoly phase_parities)
{
    // Generate Gray code
    std::vector<uint32_t> gray_code(1u << circuit.num_qubits());
    for (uint32_t i = 0; i < (1u << circuit.num_qubits()); ++i) {
        gray_code.at(i) = ((i >> 1) ^ i);
    }

    // Initialize the parity of each qubit state
    // Applying phase gate to phase_parities that consisting of just one
    // variable i is the index of the target
    std::vector<uint32_t> qubits_states(circuit.num_qubits(), 0);
    for (uint32_t i = 0u; i < circuit.num_qubits(); ++i) {
        qubits_states[i] = (1u << i);
        auto angle = phase_parities.extract_phase(qubits_states[i]);
        if (angle != 0.0) {
            circuit.apply_operator(Op::P(angle), {qubits[i]}, cbits);
        }
    }

    for (uint32_t i = circuit.num_qubits() - 1; i > 0; --i) {
        for (uint32_t j = (1u << (i + 1)) - 1; j > (1u << i); --j) {
            uint32_t c0 = std::log2(gray_code[j] ^ gray_code[j - 1u]);
            circuit.apply_operator(Op::X(), {qubits[c0], qubits[i]}, cbits);

            qubits_states[i] ^= qubits_states[c0];
            auto angle = phase_parities.extract_phase(qubits_states[i]);
            if (angle != 0.0) {
                circuit.apply_operator(Op::P(angle), {qubits[i]}, cbits);
            }
        }
        uint32_t c1 =
          std::log2(gray_code[1 << i] ^ gray_code[(1u << (i + 1)) - 1u]);
        circuit.apply_operator(Op::X(), {qubits[c1], qubits[i]}, cbits);

        qubits_states[i] ^= qubits_states[c1];
        auto angle = phase_parities.extract_phase(qubits_states[i]);
        if (angle != 0.0) {
            circuit.apply_operator(Op::P(angle), {qubits[i]}, cbits);
        }
    }
}

} // namespace

void all_linear_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, LinPhasePoly const& phase_parities)
{
    if (phase_parities.size() == 0) {
        return;
    }
    synthesize(circuit, qubits, cbits, phase_parities);
}

Circuit all_linear_synth(
  uint32_t num_qubits, LinPhasePoly const& phase_parities)
{
    Circuit circuit;
    std::vector<Qubit> qubits;
    qubits.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    all_linear_synth(circuit, qubits, {}, phase_parities);
    return circuit;
}

} // namespace tweedledum