/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/gates/gate_kinds.hpp"

#include <array>
#include <cstdint>
#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/views/topo_view.hpp>
#include <stack>
#include <vector>

namespace tweedledum {

using namespace mockturtle;

enum class mapping_strategy_action { compute, uncompute };

template<class LogicNetwork>
class bennett_mapping_strategy {
public:
	bennett_mapping_strategy(LogicNetwork const& ntk)
	{
		std::unordered_set<node<LogicNetwork>> drivers;
		ntk.foreach_po(
		    [&](auto const& f) { drivers.insert(ntk.get_node(f)); });

		auto it = steps.begin();
		topo_view view{ntk};
		view.foreach_node([&](auto n) {
			if (ntk.is_constant(n) || ntk.is_pi(n))
				return true;

			/* compute step */
			it = steps.insert(it,
			                  {n, mapping_strategy_action::compute});
			++it;

			if (!drivers.count(n))
				it = steps.insert(
				    it, {n, mapping_strategy_action::uncompute});

			return true;
		});
	}

	template<class Fn>
	inline void foreach_step(Fn&& fn) const
	{
		for (auto const& [n, a] : steps)
			fn(n, a);
	}

private:
	std::vector<std::pair<node<LogicNetwork>, mapping_strategy_action>> steps;
};

struct logic_network_synthesis_params {
	bool verbose{false};
};

namespace detail {

template<class QuantumNetwork, class LogicNetwork, class MappingStrategy>
class logic_network_synthesis_impl {
public:
	logic_network_synthesis_impl(QuantumNetwork& qnet,
	                             LogicNetwork const& ntk,
	                             logic_network_synthesis_params const& ps)
	    : qnet(qnet)
	    , ntk(ntk)
	    , ps(ps)
	    , node_to_qubit(ntk)
	{}

	void run()
	{
		prepare_inputs();
		prepare_constant(false);
		if (ntk.get_node(ntk.get_constant(false))
		    != ntk.get_node(ntk.get_constant(true)))
			prepare_constant(true);

		MappingStrategy strategy{ntk};
		strategy.foreach_step([&](auto node, auto action) {
			switch (action) {
			case mapping_strategy_action::compute: {
				if (ps.verbose)
					std::cout << "[i] compute "
					          << ntk.node_to_index(node)
					          << "\n";
				const auto t = node_to_qubit[node]
				    = request_ancilla();
				compute_node(node, t);
			} break;
			case mapping_strategy_action::uncompute: {
				if (ps.verbose)
					std::cout << "[i] uncompute "
					          << ntk.node_to_index(node)
					          << "\n";
				const auto t = node_to_qubit[node];
				compute_node(node, t);
				release_ancilla(t);
			} break;
			}
		});
	}

private:
	void prepare_inputs()
	{
		/* prepare primary inputs of logic network */
		ntk.foreach_pi([&](auto n) {
			node_to_qubit[n] = qnet.num_qubits();
			qnet.allocate_qubit();
		});
	}

	void prepare_constant(bool value)
	{
		const auto f = ntk.get_constant(value);
		const auto n = ntk.get_node(f);
		if (ntk.fanout_size(n) == 0)
			return;
		const auto v = ntk.constant_value(n) ^ ntk.is_complemented(f);
		node_to_qubit[n] = qnet.num_qubits();
		qnet.allocate_qubit();
		if (v)
			qnet.add_gate(gate_kinds_t::pauli_x, node_to_qubit[n]);
	}

	uint32_t request_ancilla()
	{
		if (free_ancillae.empty()) {
			const auto r = qnet.num_qubits();
			qnet.allocate_qubit();
			return r;
		} else {
			const auto r = free_ancillae.top();
			free_ancillae.pop();
			return r;
		}
	}

	void release_ancilla(uint32_t q)
	{
		free_ancillae.push(q);
	}

	template<int Fanin>
	std::array<uint32_t, Fanin>
	get_fanin_as_literals(node<LogicNetwork> const& n)
	{
		std::array<uint32_t, Fanin> controls;
		ntk.foreach_fanin(n, [&](auto const& f, auto i) {
			controls[i] = (ntk.node_to_index(ntk.get_node(f)) << 1)
			              | ntk.is_complemented(f);
		});
		return controls;
	}

	void compute_node(node<LogicNetwork> const& node, uint32_t t)
	{
		if (ntk.is_and(node)) {
			auto controls = get_fanin_as_literals<2>(node);
			compute_and(
			    node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
			    node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
			    controls[0] & 1, controls[1] & 1, t);
		} else if (ntk.is_xor(node)) {
			auto controls = get_fanin_as_literals<2>(node);
			compute_xor(
			    node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
			    node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
			    (controls[0] & 1) != (controls[1] & 1), t);
		} else if (ntk.is_xor3(node)) {
			auto controls = get_fanin_as_literals<3>(node);
			compute_xor3(
			    node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
			    node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
			    node_to_qubit[ntk.index_to_node(controls[2] >> 1)],
			    ((controls[0] & 1) != (controls[1] & 1))
			        != (controls[2] & 1),
			    t);
		} else if (ntk.is_maj(node)) {
			auto controls = get_fanin_as_literals<3>(node);
			compute_maj(
			    node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
			    node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
			    node_to_qubit[ntk.index_to_node(controls[2] >> 1)],
			    controls[0] & 1, controls[1] & 1, controls[2] & 1, t);
		}
	}

	void compute_and(uint32_t c1, uint32_t c2, bool p1, bool p2, uint32_t t)
	{
		if (p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
		if (p2)
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		qnet.add_multiple_controlled_gate(gate_kinds_t::mcx, {t, c1, c2});
		if (p2)
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		if (p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
	}

	void compute_xor(uint32_t c1, uint32_t c2, bool inv, uint32_t t)
	{
		qnet.add_controlled_gate(gate_kinds_t::cx, c1, t);
		qnet.add_controlled_gate(gate_kinds_t::cx, c2, t);
		if (inv)
			qnet.add_gate(gate_kinds_t::pauli_x, t);
	}

	void compute_xor3(uint32_t c1, uint32_t c2, uint32_t c3, bool inv,
	                  uint32_t t)
	{
		qnet.add_controlled_gate(gate_kinds_t::cx, c1, t);
		qnet.add_controlled_gate(gate_kinds_t::cx, c2, t);
		qnet.add_controlled_gate(gate_kinds_t::cx, c3, t);
		if (inv)
			qnet.add_gate(gate_kinds_t::pauli_x, t);
	}

	void compute_maj(uint32_t c1, uint32_t c2, uint32_t c3, bool p1,
	                 bool p2, bool p3, uint32_t t)
	{
		if (p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
		if (!p2) /* control 2 behaves opposite */
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		if (p3)
			qnet.add_gate(gate_kinds_t::pauli_x, c3);
		qnet.add_controlled_gate(gate_kinds_t::cx, c1, c2);
		qnet.add_controlled_gate(gate_kinds_t::cx, c3, c1);
		qnet.add_controlled_gate(gate_kinds_t::cx, c3, t);
		qnet.add_multiple_controlled_gate(gate_kinds_t::mcx, {t, c1, c2});
		qnet.add_controlled_gate(gate_kinds_t::cx, c3, c1);
		qnet.add_controlled_gate(gate_kinds_t::cx, c1, c2);
		if (p3)
			qnet.add_gate(gate_kinds_t::pauli_x, c3);
		if (!p2)
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		if (p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
	}

private:
	QuantumNetwork& qnet;
	LogicNetwork const& ntk;
	logic_network_synthesis_params const& ps;
	node_map<uint32_t, LogicNetwork> node_to_qubit;
	std::stack<uint32_t> free_ancillae;
}; // namespace detail

} // namespace detail

/*! \brief Hierarchical synthesis based on a logic network
 *
 * This algorithm used hierarchical synthesis and computes a reversible network
 * for each gate in the circuit and computes the intermediate result to an
 * ancilla line.  The node may be computed out-of-place or in-place.  The
 * order in which nodes are computed and uncomputed, and whether they are
 * computed out-of-place or in-place is determined by a separate mapper
 * component `MappingStrategy` that is passed as template parameter to the
 * function.
 */
template<class QuantumNetwork, class LogicNetwork,
         class MappingStrategy = bennett_mapping_strategy<LogicNetwork>>
void logic_network_synthesis(QuantumNetwork& qnet, LogicNetwork const& ntk,
                             logic_network_synthesis_params const& ps = {})
{
	static_assert(is_network_type_v<LogicNetwork>,
	              "LogicNetwork is not a network type");

	detail::logic_network_synthesis_impl<QuantumNetwork, LogicNetwork, MappingStrategy>
	    impl(qnet, ntk, ps);
	impl.run();
}

} /* namespace tweedledum */
