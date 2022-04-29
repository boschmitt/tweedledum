/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Synthesis/lhrs_synth.h"
#include "BennettStrategy.h"
#include "EagerStrategy.h"
#include "tweedledum/Operators/Extension/TruthTable.h"
#include "tweedledum/Operators/Standard/X.h"

#include <cassert>
#include <memory>
#include <mockturtle/algorithms/collapse_mapped.hpp>
#include <mockturtle/algorithms/lut_mapping.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/networks/xag.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/views/mapping_view.hpp>

namespace tweedledum {

namespace {

struct Config {
    std::unique_ptr<BaseStrategy> strategy;

    Config(nlohmann::json const& config)
        : strategy(std::make_unique<BennettStrategy>())
    {}
};

using namespace mockturtle;

inline klut_network collapse_to_klut(xag_network const& xag)
{
    lut_mapping_params ps;
    ps.cut_enumeration_ps.cut_size = 4;
    // Do LUT mapping while storing the functions.
    mapping_view<xag_network, true> mapped_xag(xag);
    lut_mapping<mapping_view<xag_network, true>, true>(mapped_xag, ps);
    // Collapse and return
    return *collapse_mapped_network<klut_network>(mapped_xag);
}

inline void synthesize(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, xag_network const& xag, Config const& config)
{
    using Action = BaseStrategy::Action;

    auto const klut = collapse_to_klut(xag);
    config.strategy->compute_steps(klut);
    node_map<Qubit, klut_network> to_qubit(klut, Qubit::invalid());

    uint32_t i = 0u;
    klut.foreach_pi([&](auto const& node) { to_qubit[node] = qubits.at(i++); });

    // Analysis of the primary outputs:  Here I do two things:
    // *) look for primary outputs that point to the same node.  For those
    //     we need to only compute one and then at the end use a CX to copy
    //     the computational state.
    // *) check which outputs will need be complemented at the end.
    klut.clear_visited();
    std::vector<uint32_t> to_compute_po;
    std::vector<uint32_t> to_complement_po;
    klut.foreach_po([&](auto const& signal) {
        auto node = klut.get_node(signal);
        if (!klut.visited(node)) {
            to_qubit[node] = qubits.at(i);
            klut.set_visited(node, 1u);
            if (klut.is_complemented(signal)) {
                to_complement_po.push_back(i);
            }
        } else {
            to_compute_po.push_back(i);
        }
        ++i;
    });

    // Perform the action of all the steps.
    for (auto const& step : *config.strategy) {
        std::vector<Qubit> qs;
        klut.foreach_fanin(step.node, [&](auto const& signal) {
            Qubit qubit = to_qubit[klut.get_node(signal)];
            if (klut.is_complemented(signal)) {
                qs.emplace_back(!qubit);
            } else {
                qs.emplace_back(qubit);
            }
        });
        switch (step.action) {
        case Action::compute:
            if (to_qubit[step.node] == Qubit::invalid()) {
                to_qubit[step.node] = circuit.request_ancilla();
            }
            break;

        case Action::cleanup:
            circuit.release_ancilla(to_qubit[step.node]);
            break;
        }
        qs.push_back(to_qubit[step.node]);
        circuit.apply_operator(
          Op::TruthTable(klut.node_function(step.node)), qs, cbits);
    }

    // Compute the outputs that need to be "copied" from other qubits.
    for (uint32_t po : to_compute_po) {
        auto const signal = klut.po_at(po - klut.num_pis());
        auto const node = klut.get_node(signal);
        Qubit qubit = to_qubit[node];
        if (klut.is_complemented(signal)) {
            qubit = !qubit;
        }
        circuit.apply_operator(Op::X(), {qubit, qubits.at(po)}, cbits);
    }
    // Complement what needs to be complemented.
    for (uint32_t po : to_complement_po) {
        auto const signal = klut.po_at(po - klut.num_pis());
        auto const node = klut.get_node(signal);
        Qubit const qubit = to_qubit[node];
        circuit.apply_operator(Op::X(), {qubit}, cbits);
    }
}

} // namespace

void lhrs_synth(Circuit& circuit, std::vector<Qubit> const& qubits,
  std::vector<Cbit> const& cbits, mockturtle::xag_network const& xag,
  nlohmann::json const& config)
{
    Config cfg(config);
    synthesize(circuit, qubits, cbits, xag, cfg);
}

//  LUT-based hierarchical reversible logic synthesis (LHRS)
Circuit lhrs_synth(
  mockturtle::xag_network const& xag, nlohmann::json const& config)
{
    Circuit circuit;
    Config cfg(config);
    uint32_t num_qubits = xag.num_pis() + xag.num_pos();
    std::vector<Qubit> qubits;
    qubits.reserve(num_qubits);
    for (uint32_t i = 0u; i < num_qubits; ++i) {
        qubits.emplace_back(circuit.create_qubit());
    }
    synthesize(circuit, qubits, {}, xag, cfg);
    return circuit;
}

} // namespace tweedledum
