/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/barenco_decomp.h"

#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

namespace tweedledum {

namespace {

struct Config {
    uint32_t controls_threshold;
    uint32_t max_qubits;
    // std::vector<uint8_t> locked;
    WireRef locked;
    Operator const compute_op;
    Operator const cleanup_op;

    Config(uint32_t n, nlohmann::json const& config)
        : controls_threshold(2u)
        , max_qubits(0u)
        // , locked(n, 0)
        , locked(WireRef::invalid())
        , compute_op(Op::Rx(sym_angle::pi))
        , cleanup_op(Op::Rx(-sym_angle::pi))
        // , compute_op(Op::X())
        // , cleanup_op(Op::X())
    {
        auto barenco_cfg = config.find("barenco_cfg");
        if (barenco_cfg != config.end()) {
            if (barenco_cfg->contains("controls_threshold")) {
                controls_threshold = barenco_cfg->at("controls_threshold");
            }
        }
        if (config.contains("max_qubits")) {
            max_qubits = config.at("max_qubits");
        }
    }
};

inline std::vector<WireRef> get_workspace(Circuit& circuit, std::vector<WireRef> const& qubits, Config const& cfg)
{
    std::vector<WireRef> workspace;
    circuit.foreach_wire([&](WireRef ref) {
        if (ref.kind() != Wire::Kind::quantum) {
            return;
        }
        // if (cfg.locked[ref]) {
        if (cfg.locked == ref) {
            return;
        }
        for (WireRef qubit : qubits) {
            if (ref.uid() == qubit.uid()) {
                return;
            }
        }
        workspace.push_back(ref);
    });
    return workspace;
}

inline void v_dirty(Circuit& circuit, Operator const& op, std::vector<WireRef> const& qubits, Config const& cfg)
{
    uint32_t const num_controls = qubits.size() - 1;
    std::vector<WireRef> workspace = get_workspace(circuit, qubits, cfg);
    uint32_t const workspace_size = workspace.size();
    workspace.push_back(qubits.back());

    // When offset is equal to 0 this is computing
    // When offset is 1 this is cleaning up the workspace
    for (int offset = 0; offset <= 1; ++offset) {
        for (int i = offset; i < static_cast<int>(num_controls) - 2; ++i) {
            WireRef const c0 = qubits.at(num_controls - 1 - i);
            WireRef const c1 = workspace.at(workspace_size - 1 - i);
            WireRef const t = workspace.at(workspace_size - i);
            circuit.apply_operator(i ? cfg.compute_op : op, {c0, c1, t});
        }

        circuit.apply_operator(offset ? cfg.cleanup_op : cfg.compute_op, {qubits[0], qubits[1], workspace[workspace_size - (num_controls - 2)]});

        for (int i = num_controls - 2 - 1; i >= offset; --i) {
            WireRef const c0 = qubits.at(num_controls - 1 - i);
            WireRef const c1 = workspace.at(workspace_size - 1 - i);
            WireRef const t = workspace.at(workspace_size - i);
            circuit.apply_operator(i ? cfg.cleanup_op : op, {c0, c1, t});
        }
    }
}

inline void v_clean(Circuit& circuit, Operator const& op, std::vector<WireRef> const& qubits, Config const& cfg)
{
    uint32_t const num_controls = qubits.size() - 1;
    std::vector<WireRef> ancillae;
    for (uint32_t i = 0; i < (num_controls - 2); ++i) {
        ancillae.push_back(circuit.request_ancilla());
    }

    circuit.apply_operator(cfg.compute_op, {qubits[0], qubits[1], ancillae.at(0)});
    uint32_t j = 0;
    for (uint32_t i = 2; i < (num_controls - 1); ++i) {
        circuit.apply_operator(cfg.compute_op, {qubits.at(i), ancillae.at(j), ancillae.at(j + 1)});
        j += 1;
    }

    circuit.apply_operator(op, {qubits.at(num_controls - 1), ancillae.back(), qubits.back()});

    for (uint32_t i = (num_controls - 1); i --> 2;) {
        circuit.apply_operator(cfg.cleanup_op, {qubits.at(i), ancillae.at(j - 1), ancillae.at(j)});
        j -= 1;
    }
    circuit.apply_operator(cfg.cleanup_op, {qubits[0], qubits[1], ancillae.at(0)});
    for (WireRef const ref : ancillae) {
        circuit.release_ancilla(ref);
    }
}

inline void dirty_ancilla(Circuit& circuit, Operator const& op, std::vector<WireRef> const& qubits, Config const& cfg)
{
    uint32_t const num_controls = qubits.size() - 1;
    if (num_controls <= cfg.controls_threshold) {
        circuit.apply_operator(op, qubits);
        return;
    }

    // Check if there are enough unused lines
    // Lemma 7.2: If n ≥ 5 and m ∈ {3, ..., ⌈n/2⌉} then a gate can be simulated
    // by a circuit consisting of 4(m − 2) gates.
    // n is the number of qubits
    // m is the number of controls
    if (circuit.num_qubits() + 1 >= (num_controls << 1)) {
        v_dirty(circuit, op, qubits, cfg);
        return;
    }

    std::vector<WireRef> workspace = get_workspace(circuit, qubits, cfg);
    // Not enough qubits in the workspace, extra decomposition step
    // Lemma 7.3: For any n ≥ 5, and m ∈ {2, ... , n − 3} a (n−2)-toffoli gate
    // can be simulated by a circuit consisting of two m-toffoli gates and two
    // (n−m−1)-toffoli gates
    std::vector<WireRef> qubits0;
    std::vector<WireRef> qubits1;
    for (auto i = 0u; i < (num_controls >> 1); ++i) {
        qubits0.push_back(qubits[i]);
    }
    for (auto i = (num_controls >> 1); i < num_controls; ++i) {
        qubits1.push_back(qubits[i]);
    }
    WireRef free_qubit = workspace.front();
    qubits0.push_back(free_qubit);
    qubits1.push_back(free_qubit);
    qubits1.push_back(qubits.back());
    dirty_ancilla(circuit, cfg.compute_op, qubits0, cfg);
    dirty_ancilla(circuit, op, qubits1, cfg);
    dirty_ancilla(circuit, cfg.cleanup_op, qubits0, cfg);
    dirty_ancilla(circuit, op, qubits1, cfg);
}

inline void clean_ancilla(Circuit& circuit, Operator const& op, std::vector<WireRef> const& qubits, Config& cfg)
{
    uint32_t const num_controls = qubits.size() - 1;
    if (num_controls <= cfg.controls_threshold) {
        circuit.apply_operator(op, qubits);
        return;
    }

    // Check if there are enough unused lines
    // Lemma 7.2: If n ≥ 5 and m ∈ {3, ..., ⌈n/2⌉} then a gate can be simulated
    // by a circuit consisting of 4(m − 2) gates.
    // n is the number of qubits
    // m is the number of controls
    if (circuit.num_qubits() + 1 >= (num_controls << 1)) {
        v_dirty(circuit, op, qubits, cfg);
        return;
    }

    // Not enough qubits in the workspace, extra decomposition step
    // Lemma 7.3: For any n ≥ 5, and m ∈ {2, ... , n − 3} a (n−2)-toffoli gate
    // can be simulated by a circuit consisting of two m-toffoli gates and two
    // (n−m−1)-toffoli gates
    std::vector<WireRef> qubits0;
    std::vector<WireRef> qubits1;
    for (auto i = 0u; i < (num_controls >> 1); ++i) {
        qubits0.push_back(qubits[i]);
    }
    for (auto i = (num_controls >> 1); i < num_controls; ++i) {
        qubits1.push_back(qubits[i]);
    }
    if (circuit.num_ancillae() > 0) {
        // fmt::print("b\n");
        WireRef ancilla = circuit.request_ancilla();
        // cfg.locked[ancilla] = 1;
        qubits0.push_back(ancilla);
        qubits1.push_back(ancilla);
        qubits1.push_back(qubits.back());
        clean_ancilla(circuit, cfg.compute_op, qubits0, cfg);
        clean_ancilla(circuit, op, qubits1, cfg);
        clean_ancilla(circuit, cfg.cleanup_op, qubits0, cfg);
        circuit.release_ancilla(ancilla);
        return;
    }
    std::vector<WireRef> workspace = get_workspace(circuit, qubits, cfg);
    WireRef free_qubit = workspace.front();
    qubits0.push_back(free_qubit);
    qubits1.push_back(free_qubit);
    qubits1.push_back(qubits.back());
    dirty_ancilla(circuit, cfg.compute_op, qubits0, cfg);
    dirty_ancilla(circuit, op, qubits1, cfg);
    dirty_ancilla(circuit, cfg.cleanup_op, qubits0, cfg);
    dirty_ancilla(circuit, op, qubits1, cfg);
}

inline bool decompose(Circuit& circuit, Instruction const& inst, Config& cfg)
{
    // cfg.locked[inst.target()] = 1;
    cfg.locked = inst.target();
    if (inst.num_qubits() == circuit.num_qubits()) {
        circuit.create_ancilla();
        // cfg.locked.push_back(0);
    }
    if (inst.num_controls() == 3u && circuit.num_ancillae() > 0) {
        WireRef ancilla = circuit.request_ancilla();
        circuit.apply_operator(cfg.compute_op, {inst.control(0), inst.control(1), ancilla});
        circuit.apply_operator(inst, {inst.control(2), ancilla, inst.target()});
        circuit.apply_operator(cfg.cleanup_op, {inst.control(0), inst.control(1), ancilla});
        circuit.release_ancilla(ancilla);
        return true;
    }
    if (circuit.num_ancillae() == 0 && circuit.num_qubits() >= cfg.max_qubits) {
        dirty_ancilla(circuit, inst, inst.qubits(), cfg);
        return true;
    }
    for (uint32_t i = circuit.num_qubits(); i < cfg.max_qubits && circuit.num_ancillae() < (inst.num_controls() - 2); ++i) {
        circuit.create_ancilla();
    }
    if (circuit.num_ancillae() >= (inst.num_controls() - 2)) {
        v_clean(circuit, inst, inst.qubits(), cfg);
    } else {
        clean_ancilla(circuit, inst, inst.qubits(), cfg);
    }
    return true;
}

}

void barenco_decomp(Circuit& circuit, Instruction const& inst, nlohmann::json const& config)
{
    Config cfg(circuit.num_qubits(), config);
    decompose(circuit, inst, cfg);
}

void barenco_decomp(Circuit& circuit, Circuit const& original, nlohmann::json const& config)
{
    Config cfg(original.num_qubits(), config);
    original.foreach_instruction([&](Instruction const& inst) {
        // Only X, Y, Z
        if (inst.is_one<Op::X, Op::Y, Op::Z>() && inst.num_controls() > cfg.controls_threshold) {
            decompose(circuit, inst, cfg);
            return;
        }
        circuit.apply_operator(inst);
    });
}

Circuit barenco_decomp(Circuit const& original, nlohmann::json const& config)
{
    Config cfg(original.num_qubits(), config);
    Circuit result = shallow_duplicate(original);
    original.foreach_instruction([&](Instruction const& inst) {
        // Only X, Y, Z
        if (inst.is_one<Op::X, Op::Y, Op::Z>() && inst.num_controls() > cfg.controls_threshold) {
            decompose(result, inst, cfg);
            return;
        }
        result.apply_operator(inst);
    });
    return result;
}

} // namespace tweedledum
