/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Operators/Extension/TruthTable.h"
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Synthesis/pkrm_synth.h"
#include "tweedledum/Utils/Classical/xag_simulate.h"

#include <algorithm>
#include <cassert>
#include <kitty/kitty.hpp>
#include <mockturtle/algorithms/simulation.hpp>

namespace tweedledum {

namespace {

struct Config {
    bool phase_esop;

    Config(nlohmann::json const& config)
        : phase_esop(false)
    {
        auto cfg = config.find("pkrm_synth");
        if (cfg != config.end()) {
            if (cfg->contains("phase_esop")) {
                phase_esop = cfg->at("phase_esop");
            }
        }
    }
};

inline void synthesize(Circuit& circuit, std::vector<Qubit> const& qubits,
    std::vector<Cbit> const& cbits, kitty::dynamic_truth_table const& function,
    Config const& config)
{
    Qubit const target = qubits.back();
    for (auto const& cube : kitty::esop_from_optimum_pkrm(function)) {
        uint32_t bits = cube._bits;
        uint32_t mask = cube._mask;
        std::vector<Qubit> qs;
        for (uint32_t v = 0u; mask; mask >>= 1, bits >>= 1, ++v) {
            if ((mask & 1) == 0u) {
                continue;
            }
            qs.push_back((bits & 1) ? qubits.at(v) : !qubits.at(v));
        }
        if (config.phase_esop) {
            auto it = std::find_if(qs.rbegin(), qs.rend(), 
            [](Qubit qubit) {
                return qubit.polarity() == Qubit::Polarity::positive;
            });
            if (it == qs.rend()) {
                circuit.apply_operator(Op::X(), {qs.back()}, cbits);
                circuit.apply_operator(Op::Z(), qs, cbits);
                circuit.apply_operator(Op::X(), {qs.back()}, cbits);
            } else {
                std::swap(*it, qs.back());
                circuit.apply_operator(Op::Z(), qs, cbits);
            }
        } else {
            qs.push_back(target);
            circuit.apply_operator(Op::X(), qs, cbits);
        }
    }
}

}

void pkrm_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
     std::vector<Cbit> const& cbits, kitty::dynamic_truth_table const& function,
     nlohmann::json const& config)
{
    Config cfg(config);
    assert(cfg.phase_esop ? qubits.size() == function.num_vars()
                          : qubits.size() == (function.num_vars() + 1));
    synthesize(circuit, qubits, cbits, function, cfg);
}

Circuit pkrm_synth(kitty::dynamic_truth_table const& function,
    nlohmann::json const& config)
{
    Circuit circuit;
    Config cfg(config);
    std::vector<Qubit> qubits;
    qubits.reserve(function.num_vars() + 1);
    for (uint32_t i = 0u; i < function.num_vars(); ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    if (!cfg.phase_esop) {
        qubits.emplace_back(circuit.create_qubit());
    }
    synthesize(circuit, qubits, {}, function, cfg);
    return circuit;
}

// void pkrm_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
//     Instruction const& inst, nlohmann::json const& config)
// {
//     if (!inst.is_a<Op::TruthTable>()) {
//         return;
//     }
//     auto tt = inst.cast<Op::TruthTable>();
//     pkrm_synth(circuit, qubits, inst.cbits(), tt.truth_table(), config);
// }

// void pkrm_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
//     mockturtle::xag_network const& xag, nlohmann::json const& config)
// {
//     if (xag.num_pis() <= 16u && xag.num_pos() == 1) {
//         auto tts = xag_simulate(xag);
//         pkrm_synth(circuit, qubits, tts.at(0), config);
//         return;
//     }
// }

// Circuit pkrm_synth(mockturtle::xag_network const& xag, nlohmann::json const& config)
// {
//     if (xag.num_pis() <= 16u && xag.num_pos() == 1) {
//         auto tts = xag_simulate(xag);
//         return pkrm_synth(tts.at(0), config);
//     }
//     Circuit circuit;
//     return circuit;
// }

} // namespace tweedledum
