/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Decomposition/BarencoDecomposer.h"

#include "tweedledum/Operators/Standard.h"
#include "tweedledum/Passes/Utility/shallow_duplicate.h"

namespace tweedledum {

BarencoDecomposer::Config::Config(nlohmann::json const& config)
    : controls_threshold(2u)
    , max_qubits(0u)
    , locked(Qubit::invalid())
    , compute_op(Op::Rx(numbers::pi))
    , cleanup_op(Op::Rx(-numbers::pi))
{
    auto barenco_cfg = config.find("barenco_decomp");
    if (barenco_cfg != config.end()) {
        if (barenco_cfg->contains("controls_threshold")) {
            controls_threshold = barenco_cfg->at("controls_threshold");
        }
        if (barenco_cfg->contains("use_relative_phase")) {
            bool use_relative_phase = barenco_cfg->at("use_relative_phase");
            if (!use_relative_phase) {
                compute_op = Op::X();
                cleanup_op = Op::X();
            }
        }
    }
    if (config.contains("max_qubits")) {
        max_qubits = config.at("max_qubits");
    }
}

std::vector<Qubit> BarencoDecomposer::get_workspace(
  Circuit& circuit, std::vector<Qubit> const& qubits)
{
    std::vector<Qubit> workspace;
    circuit.foreach_qubit([&](Qubit current_qubit) {
        if (config.locked == current_qubit) {
            return;
        }
        for (Qubit qubit : qubits) {
            if (current_qubit.uid() == qubit.uid()) {
                return;
            }
        }
        workspace.push_back(current_qubit);
    });
    return workspace;
}

void BarencoDecomposer::v_dirty(Circuit& circuit, Operator const& op,
  std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits)
{
    uint32_t const num_controls = qubits.size() - 1;
    std::vector<Qubit> workspace = get_workspace(circuit, qubits);
    uint32_t const workspace_size = workspace.size();
    workspace.push_back(qubits.back());

    // When offset is equal to 0 this is computing
    // When offset is 1 this is cleaning up the workspace
    for (int offset = 0; offset <= 1; ++offset) {
        for (int i = offset; i < static_cast<int>(num_controls) - 2; ++i) {
            Qubit const c0 = qubits.at(num_controls - 1 - i);
            Qubit const c1 = workspace.at(workspace_size - 1 - i);
            Qubit const t = workspace.at(workspace_size - i);
            circuit.apply_operator(
              i ? config.compute_op : op, {c0, c1, t}, cbits);
        }

        circuit.apply_operator(offset ? config.cleanup_op : config.compute_op,
          {qubits[0], qubits[1],
            workspace[workspace_size - (num_controls - 2)]},
          cbits);

        for (int i = num_controls - 2 - 1; i >= offset; --i) {
            Qubit const c0 = qubits.at(num_controls - 1 - i);
            Qubit const c1 = workspace.at(workspace_size - 1 - i);
            Qubit const t = workspace.at(workspace_size - i);
            circuit.apply_operator(
              i ? config.cleanup_op : op, {c0, c1, t}, cbits);
        }
    }
}

void BarencoDecomposer::v_clean(Circuit& circuit, Operator const& op,
  std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits)
{
    uint32_t const num_controls = qubits.size() - 1;
    std::vector<Qubit> ancillae;
    for (uint32_t i = 0; i < (num_controls - 2); ++i) {
        ancillae.push_back(circuit.request_ancilla());
    }

    circuit.apply_operator(
      config.compute_op, {qubits[0], qubits[1], ancillae.at(0)});
    uint32_t j = 0;
    for (uint32_t i = 2; i < (num_controls - 1); ++i) {
        circuit.apply_operator(config.compute_op,
          {qubits.at(i), ancillae.at(j), ancillae.at(j + 1)}, cbits);
        j += 1;
    }

    circuit.apply_operator(
      op, {qubits.at(num_controls - 1), ancillae.back(), qubits.back()}, cbits);

    for (uint32_t i = (num_controls - 1); i-- > 2;) {
        circuit.apply_operator(config.cleanup_op,
          {qubits.at(i), ancillae.at(j - 1), ancillae.at(j)}, cbits);
        j -= 1;
    }
    circuit.apply_operator(
      config.cleanup_op, {qubits[0], qubits[1], ancillae.at(0)}, cbits);
    for (Qubit& qubit : ancillae) {
        circuit.release_ancilla(qubit);
    }
}

void BarencoDecomposer::dirty_ancilla(Circuit& circuit, Operator const& op,
  std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits)
{
    uint32_t const num_controls = qubits.size() - 1;
    if (num_controls <= config.controls_threshold) {
        circuit.apply_operator(op, qubits, cbits);
        return;
    }

    // Check if there are enough unused lines
    // Lemma 7.2: If n ≥ 5 and m ∈ {3, ..., ⌈n/2⌉} then a gate can be simulated
    // by a circuit consisting of 4(m − 2) gates.
    // n is the number of qubits
    // m is the number of controls
    if (circuit.num_qubits() + 1 >= (num_controls << 1)) {
        v_dirty(circuit, op, qubits, cbits);
        return;
    }

    std::vector<Qubit> workspace = get_workspace(circuit, qubits);
    // Not enough qubits in the workspace, extra decomposition step
    // Lemma 7.3: For any n ≥ 5, and m ∈ {2, ... , n − 3} a (n−2)-toffoli gate
    // can be simulated by a circuit consisting of two m-toffoli gates and two
    // (n−m−1)-toffoli gates
    std::vector<Qubit> qubits0;
    std::vector<Qubit> qubits1;
    for (auto i = 0u; i < (num_controls >> 1); ++i) {
        qubits0.push_back(qubits[i]);
    }
    for (auto i = (num_controls >> 1); i < num_controls; ++i) {
        qubits1.push_back(qubits[i]);
    }
    Qubit free_qubit = workspace.front();
    qubits0.push_back(free_qubit);
    qubits1.push_back(free_qubit);
    qubits1.push_back(qubits.back());
    dirty_ancilla(circuit, config.compute_op, qubits0, cbits);
    dirty_ancilla(circuit, op, qubits1, cbits);
    dirty_ancilla(circuit, config.cleanup_op, qubits0, cbits);
    dirty_ancilla(circuit, op, qubits1, cbits);
}

void BarencoDecomposer::clean_ancilla(Circuit& circuit, Operator const& op,
  std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits)
{
    uint32_t const num_controls = qubits.size() - 1;
    if (num_controls <= config.controls_threshold) {
        circuit.apply_operator(op, qubits);
        return;
    }

    // Check if there are enough unused lines
    // Lemma 7.2: If n ≥ 5 and m ∈ {3, ..., ⌈n/2⌉} then a gate can be simulated
    // by a circuit consisting of 4(m − 2) gates.
    // n is the number of qubits
    // m is the number of controls
    if (!circuit.num_ancillae()
        && circuit.num_qubits() + 1 >= (num_controls << 1)) {
        v_dirty(circuit, op, qubits, cbits);
        return;
    }

    // Not enough qubits in the workspace, extra decomposition step
    // Lemma 7.3: For any n ≥ 5, and m ∈ {2, ... , n − 3} a (n−2)-toffoli gate
    // can be simulated by a circuit consisting of two m-toffoli gates and two
    // (n−m−1)-toffoli gates
    std::vector<Qubit> qubits0;
    std::vector<Qubit> qubits1;
    for (auto i = 0u; i < (num_controls >> 1); ++i) {
        qubits0.push_back(qubits[i]);
    }
    for (auto i = (num_controls >> 1); i < num_controls; ++i) {
        qubits1.push_back(qubits[i]);
    }
    if (circuit.num_ancillae() > 0) {
        Qubit ancilla = circuit.request_ancilla();
        qubits0.push_back(ancilla);
        qubits1.push_back(ancilla);
        qubits1.push_back(qubits.back());
        clean_ancilla(circuit, config.compute_op, qubits0, cbits);
        clean_ancilla(circuit, op, qubits1, cbits);
        clean_ancilla(circuit, config.cleanup_op, qubits0, cbits);
        circuit.release_ancilla(ancilla);
        return;
    }
    std::vector<Qubit> workspace = get_workspace(circuit, qubits);
    Qubit free_qubit = workspace.front();
    qubits0.push_back(free_qubit);
    qubits1.push_back(free_qubit);
    qubits1.push_back(qubits.back());
    dirty_ancilla(circuit, config.compute_op, qubits0, cbits);
    dirty_ancilla(circuit, op, qubits1, cbits);
    dirty_ancilla(circuit, config.cleanup_op, qubits0, cbits);
    dirty_ancilla(circuit, op, qubits1, cbits);
}

bool BarencoDecomposer::decompose(
  Circuit& circuit, Instruction const& inst)
{
    config.locked = inst.target();
    if (inst.num_qubits() == circuit.num_qubits()) {
        circuit.create_ancilla();
    }
    if (inst.num_controls() == 3u && circuit.num_ancillae() > 0) {
        Qubit ancilla = circuit.request_ancilla();
        circuit.apply_operator(config.compute_op,
          {inst.control(0), inst.control(1), ancilla}, inst.cbits());
        circuit.apply_operator(
          inst, {inst.control(2), ancilla, inst.target()}, inst.cbits());
        circuit.apply_operator(config.cleanup_op,
          {inst.control(0), inst.control(1), ancilla}, inst.cbits());
        circuit.release_ancilla(ancilla);
        return true;
    }
    if (circuit.num_ancillae() == 0
        && circuit.num_qubits() >= config.max_qubits) {
        dirty_ancilla(circuit, inst, inst.qubits(), inst.cbits());
        return true;
    }
    for (uint32_t i = circuit.num_qubits();
         i < config.max_qubits
         && circuit.num_ancillae() < (inst.num_controls() - 2);
         ++i)
    {
        circuit.create_ancilla();
    }
    if (circuit.num_ancillae() >= (inst.num_controls() - 2)) {
        v_clean(circuit, inst, inst.qubits(), inst.cbits());
    } else {
        clean_ancilla(circuit, inst, inst.qubits(), inst.cbits());
    }
    return true;
}

} // namespace tweedledum
