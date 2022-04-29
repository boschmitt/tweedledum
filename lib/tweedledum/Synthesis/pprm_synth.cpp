/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/pprm_synth.h"
#include "tweedledum/Operators/Standard.h"

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

inline void synthesize(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, kitty::dynamic_truth_table const& function,
  Config config)
{
    std::vector<Qubit> qs;
    qs.reserve(qubits.size());
    Qubit const target = qubits.back();
    for (auto const& cube : kitty::esop_from_pprm(function)) {
        auto bits = cube._bits;
        for (uint32_t v = 0u; bits; bits >>= 1, ++v) {
            if ((bits & 1) == 0u) {
                continue;
            }
            qs.push_back(qubits.at(v));
        }
        if (config.phase_esop) {
            if (qs.empty()) {
                // We have the constant 1 as a cube.  A phase oracle is not
                // capable of differentiating between f and (1 ^ f), meaning
                // that this basically adds a global phase to the circuit
                circuit.global_phase() += numbers::pi;
                // pi seems like a logical choice, but I'm _not_ 100% sure about
                // this!
            } else {
                circuit.apply_operator(Op::Z(), qs, cbits);
            }
        } else {
            qs.push_back(target);
            circuit.apply_operator(Op::X(), qs, cbits);
        }
        qs.clear();
    }
}

} // namespace

void pprm_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, kitty::dynamic_truth_table const& function,
  nlohmann::json const& config)
{
    Config cfg(config);
    assert(cfg.phase_esop ? qubits.size() == function.num_vars()
                          : qubits.size() == (function.num_vars() + 1));
    synthesize(circuit, qubits, cbits, function, cfg);
}

Circuit pprm_synth(
  kitty::dynamic_truth_table const& function, nlohmann::json const& config)
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

} // namespace tweedledum
