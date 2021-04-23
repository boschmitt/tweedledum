/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/spectrum_synth.h"

#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Synthesis/all_linear_synth.h"
#include "tweedledum/Synthesis/gray_synth.h"

namespace tweedledum {

void spectrum_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
    std::vector<Cbit> const& cbits, kitty::dynamic_truth_table const& function,
    nlohmann::json const& config)
{
    uint32_t const num_controls = function.num_vars();
    assert(qubits.size() >= (num_controls + 1u));

    auto extended_f = kitty::extend_to(function, num_controls + 1);
    auto g = extended_f.construct();
    kitty::create_nth_var(g, num_controls);
    extended_f &= g;

    LinPhasePoly parities;
    double const norm = numbers::pi / (1 << extended_f.num_vars());
    auto const spectrum = kitty::rademacher_walsh_spectrum(extended_f);
    for (uint32_t i = 1u; i < spectrum.size(); ++i) {
        if (spectrum[i] == 0) {
            continue;
        }
        parities.add_term(i, norm * spectrum[i]);
    }
    circuit.apply_operator(Op::H(), {qubits.back()}, cbits);
    if (parities.size() == spectrum.size() - 1) {
        all_linear_synth(circuit, qubits, cbits, parities);
    } else {
        gray_synth(circuit, qubits, cbits, 
            BMatrix::Identity(qubits.size(), qubits.size()), parities, config);
    }
    circuit.apply_operator(Op::H(), {qubits.back()}, cbits);
}

Circuit spectrum_synth(kitty::dynamic_truth_table const& function,
    nlohmann::json const& config)
{
    Circuit circuit;
    std::vector<Qubit> qubits;
    qubits.reserve(function.num_vars() + 1);
    for (uint32_t i = 0u; i < function.num_vars() + 1; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    spectrum_synth(circuit, qubits, {}, function, config);
    return circuit;
}

} // namespace tweedledum
