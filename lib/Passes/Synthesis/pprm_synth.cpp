/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Passes/Synthesis/pprm_synth.h"

#include <cassert>

namespace tweedledum {

namespace {

struct Config {
    bool phase_esop;

    Config(nlohmann::json const& config)
        : phase_esop(false)
    {
        auto cfg = config.find("pprm_synth");
        if (cfg != config.end()) {
            if (cfg->contains("phase_esop")) {
                phase_esop = cfg->at("phase_esop");
            }
        }
    }
};

inline void synthesize(Circuit& circuit, std::vector<WireRef> const& qubits,
    kitty::dynamic_truth_table const& function, Config config)
{
    std::vector<WireRef> wires;
    wires.reserve(qubits.size());
    WireRef const target = qubits.back();
    for (auto const& cube : kitty::esop_from_pprm(function)) {
        auto bits = cube._bits;
        for (uint32_t v = 0u; bits; bits >>= 1, ++v) {
            if ((bits & 1) == 0u) {
                continue;
            }
            wires.push_back(qubits.at(v));
        }
        if (config.phase_esop) {
            circuit.apply_operator(Op::Z(), wires);
        } else {
            wires.push_back(target);
            circuit.apply_operator(Op::X(), wires);
        }
        wires.clear();
    }
}

}

void pprm_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    kitty::dynamic_truth_table const& function, nlohmann::json const& config)
{
    Config cfg(config);
    assert(cfg.phase_esop ? qubits.size() == function.num_vars()
                          : qubits.size() == (function.num_vars() + 1));
    synthesize(circuit, qubits, function, cfg);
}

Circuit pprm_synth(kitty::dynamic_truth_table const& function, nlohmann::json const& config)
{
    Circuit circuit;
    Config cfg(config);

    std::vector<WireRef> wires;
    wires.reserve(function.num_vars() + 1);
    for (uint32_t i = 0u; i < function.num_vars(); ++i) {
        wires.emplace_back(circuit.create_qubit());
    }
    if (!cfg.phase_esop) {
        wires.emplace_back(circuit.create_qubit());
    }
    synthesize(circuit, wires, function, cfg);
    return circuit;
}

} // namespace tweedledum
