/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_kinds.hpp"

#include <array>
#include <cstdint>
#include <mockturtle/traits.hpp>
#include <mockturtle/utils/node_map.hpp>
#include <mockturtle/views/topo_view.hpp>
#include <stack>
#include <variant>
#include <vector>

namespace tweedledum {

namespace detail {
template<class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...)->overloaded<Ts...>;
} // namespace detail

namespace mt = mockturtle;

struct compute_action {};
struct uncompute_action {};
struct compute_inplace_action {
	uint32_t target_index;
};
struct uncompute_inplace_action {
	uint32_t target_index;
};

using mapping_strategy_action
    = std::variant<compute_action, uncompute_action, compute_inplace_action, uncompute_inplace_action>;

template<class LogicNetwork>
class bennett_mapping_strategy {
public:
	bennett_mapping_strategy(LogicNetwork const& ntk)
	{
		// clang-format off
		static_assert(mt::is_network_type_v<LogicNetwork>, "LogicNetwork is not a network type");
		static_assert(mt::has_foreach_po_v<LogicNetwork>, "LogicNetwork does not implement the foreach_po method");
		static_assert(mt::has_is_constant_v<LogicNetwork>, "LogicNetwork does not implement the is_constant method");
		static_assert(mt::has_is_pi_v<LogicNetwork>, "LogicNetwork does not implement the is_pi method");
		static_assert(mt::has_get_node_v<LogicNetwork>, "LogicNetwork does not implement the get_node method");
		// clang-format on

		std::unordered_set<mt::node<LogicNetwork>> drivers;
		ntk.foreach_po([&](auto const& f) { drivers.insert(ntk.get_node(f)); });

		auto it = steps.begin();
		mt::topo_view view{ntk};
		view.foreach_node([&](auto n) {
			if (ntk.is_constant(n) || ntk.is_pi(n))
				return true;

			/* compute step */
			it = steps.insert(it, {n, compute_action{}});
			++it;

			if (!drivers.count(n))
				it = steps.insert(it, {n, uncompute_action{}});

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
	std::vector<std::pair<mt::node<LogicNetwork>, mapping_strategy_action>> steps;
};

template<class LogicNetwork>
class bennett_mapping_inplace_strategy {
public:
	bennett_mapping_inplace_strategy(LogicNetwork const& ntk)
	{
		// clang-format off
		static_assert(mt::is_network_type_v<LogicNetwork>, "LogicNetwork is not a network type");
		static_assert(mt::has_foreach_po_v<LogicNetwork>, "LogicNetwork does not implement the foreach_po method");
		static_assert(mt::has_is_constant_v<LogicNetwork>, "LogicNetwork does not implement the is_constant method");
		static_assert(mt::has_is_pi_v<LogicNetwork>, "LogicNetwork does not implement the is_pi method");
		static_assert(mt::has_get_node_v<LogicNetwork>, "LogicNetwork does not implement the get_node method");
		static_assert(mt::has_node_to_index_v<LogicNetwork>, "LogicNetwork does not implement the node_to_index method");
		static_assert(mt::has_clear_values_v<LogicNetwork>, "LogicNetwork does not implement the clear_values method");
		static_assert(mt::has_set_value_v<LogicNetwork>, "LogicNetwork does not implement the set_value method");
		static_assert(mt::has_decr_value_v<LogicNetwork>, "LogicNetwork does not implement the decr_value method");
		static_assert(mt::has_fanout_size_v<LogicNetwork>, "LogicNetwork does not implement the fanout_size method");
		static_assert(mt::has_foreach_fanin_v<LogicNetwork>, "LogicNetwork does not implement the foreach_fanin method");
		// clang-format on

		std::unordered_set<mt::node<LogicNetwork>> drivers;
		ntk.foreach_po([&](auto const& f) { drivers.insert(ntk.get_node(f)); });

		ntk.clear_values();
		ntk.foreach_node([&](const auto& n) { ntk.set_value(n, ntk.fanout_size(n)); });

		auto it = steps.begin();
		mt::topo_view view{ntk};
		view.foreach_node([&](auto n) {
			if (ntk.is_constant(n) || ntk.is_pi(n))
				return true;

			/* decrease reference counts and mark potential target for inplace */
			int target{-1};
			ntk.foreach_fanin(n, [&](auto f) {
				if (ntk.decr_value(ntk.get_node(f)) == 0) {
					if (target != -1) {
						target = ntk.node_to_index(ntk.get_node(f));
					}
				}
			});

			/* check for inplace (only if there is possible target and node is not an output driver) */
			if (target != -1 && !drivers.count(n)) {
				if constexpr (mt::has_is_xor_v<LogicNetwork>) {
					if (ntk.is_xor(n)) {
						it = steps.insert(it, {n, compute_inplace_action{
						                              target}});
						++it;
						it = steps.insert(it, {n, uncompute_inplace_action{
						                              target}});
						return true;
					}
				}
				if constexpr (mt::has_is_xor3_v<LogicNetwork>) {
					if (ntk.is_xor3(n)) {
						it = steps.insert(it, {n, compute_inplace_action{
						                              target}});
						++it;
						it = steps.insert(it, {n, uncompute_inplace_action{
						                              target}});
						return true;
					}
				}
			}

			/* compute step */
			it = steps.insert(it, {n, compute_action{}});
			++it;

			if (!drivers.count(n))
				it = steps.insert(it, {n, uncompute_action{}});

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
	std::vector<std::pair<mt::node<LogicNetwork>, mapping_strategy_action>> steps;
};

struct logic_network_synthesis_params {
	bool verbose{false};
};

namespace detail {

template<class QuantumNetwork, class LogicNetwork, class MappingStrategy>
class logic_network_synthesis_impl {
public:
	logic_network_synthesis_impl(QuantumNetwork& qnet, LogicNetwork const& ntk,
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
		if (ntk.get_node(ntk.get_constant(false)) != ntk.get_node(ntk.get_constant(true)))
			prepare_constant(true);

		MappingStrategy strategy{ntk};
		strategy.foreach_step([&](auto node, auto action) {
			std::visit(overloaded{[](auto arg) {},
			                      [&](compute_action const&) {
				                      if (ps.verbose)
					                      std::cout << "[i] compute "
					                                << ntk.node_to_index(node)
					                                << "\n";
				                      const auto t = node_to_qubit[node]
				                          = request_ancilla();
				                      compute_node(node, t);
			                      },
			                      [&](uncompute_action const&) {
				                      if (ps.verbose)
					                      std::cout << "[i] uncompute "
					                                << ntk.node_to_index(node)
					                                << "\n";
				                      const auto t = node_to_qubit[node];
				                      compute_node(node, t);
				                      release_ancilla(t);
			                      }},
			           action);
		});
	}

private:
	void prepare_inputs()
	{
		/* prepare primary inputs of logic network */
		ntk.foreach_pi([&](auto n) {
			node_to_qubit[n] = qnet.num_qubits();
			qnet.add_qubit();
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
		qnet.add_qubit();
		if (v)
			qnet.add_gate(gate_kinds_t::pauli_x, node_to_qubit[n]);
	}

	uint32_t request_ancilla()
	{
		if (free_ancillae.empty()) {
			const auto r = qnet.num_qubits();
			qnet.add_qubit();
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
	std::array<uint32_t, Fanin> get_fanin_as_literals(mt::node<LogicNetwork> const& n)
	{
		std::array<uint32_t, Fanin> controls;
		ntk.foreach_fanin(n, [&](auto const& f, auto i) {
			controls[i] = (ntk.node_to_index(ntk.get_node(f)) << 1)
			              | ntk.is_complemented(f);
		});
		return controls;
	}

	std::vector<uint32_t> get_fanin_as_qubits(mt::node<LogicNetwork> const& n)
	{
		std::vector<uint32_t> controls;
		ntk.foreach_fanin(n, [&](auto const& f, auto i) {
			assert(!ntk.is_complemented(f));
			controls.push_back(node_to_qubit[ntk.node_to_index(ntk.get_node(f))]);
		});
		return controls;
	}

	void compute_node(mt::node<LogicNetwork> const& node, uint32_t t)
	{
		if constexpr (mt::has_is_and_v<LogicNetwork>) {
			if (ntk.is_and(node)) {
				auto controls = get_fanin_as_literals<2>(node);
				compute_and(node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
				            node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
				            controls[0] & 1, controls[1] & 1, t);
				return;
			}
		}
		if constexpr (mt::has_is_or_v<LogicNetwork>) {
			if (ntk.is_or(node)) {
				auto controls = get_fanin_as_literals<2>(node);
				compute_or(node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
				           node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
				           controls[0] & 1, controls[1] & 1, t);
				return;
			}
		}
		if constexpr (mt::has_is_xor_v<LogicNetwork>) {
			if (ntk.is_xor(node)) {
				auto controls = get_fanin_as_literals<2>(node);
				compute_xor(node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
				            node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
				            (controls[0] & 1) != (controls[1] & 1), t);
				return;
			}
		}
		if constexpr (mt::has_is_xor3_v<LogicNetwork>) {
			if (ntk.is_xor3(node)) {
				auto controls = get_fanin_as_literals<3>(node);

				/* Is XOR3 in fact an XOR2? */
				if (ntk.is_constant(ntk.index_to_node(controls[0] >> 1))) {
					compute_xor(node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
					            node_to_qubit[ntk.index_to_node(controls[2] >> 1)],
					            ((controls[0] & 1) != (controls[1] & 1))
					                != (controls[2] & 1),
					            t);
				} else {
					compute_xor3(
					    node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
					    node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
					    node_to_qubit[ntk.index_to_node(controls[2] >> 1)],
					    ((controls[0] & 1) != (controls[1] & 1))
					        != (controls[2] & 1),
					    t);
				}
				return;
			}
		}
		if constexpr (mt::has_is_maj_v<LogicNetwork>) {
			if (ntk.is_maj(node)) {
				auto controls = get_fanin_as_literals<3>(node);
				/* Is XOR3 in fact an AND or OR? */
				if (ntk.is_constant(ntk.index_to_node(controls[0] >> 1))) {
					if (controls[0] & 1) {
						compute_or(
						    node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
						    node_to_qubit[ntk.index_to_node(controls[2] >> 1)],
						    controls[1] & 1, controls[2] & 1, t);
					} else {
						compute_and(
						    node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
						    node_to_qubit[ntk.index_to_node(controls[2] >> 1)],
						    controls[1] & 1, controls[2] & 1, t);
					}
				} else {
					compute_maj(
					    node_to_qubit[ntk.index_to_node(controls[0] >> 1)],
					    node_to_qubit[ntk.index_to_node(controls[1] >> 1)],
					    node_to_qubit[ntk.index_to_node(controls[2] >> 1)],
					    controls[0] & 1, controls[1] & 1, controls[2] & 1, t);
				}
				return;
			}
		}
		if constexpr (mt::has_node_function_v<LogicNetwork>) {
			// In this case, the procedure works a bit different and retrieves the
			// controls directly as mapped qubits.  We assume that the inputs cannot
			// be complemented, e.g., in the case of k-LUT networks.
			const auto controls = get_fanin_as_qubits(node);
			compute_lut(ntk.node_function(node), controls, t);
		}
	}

	void compute_and(uint32_t c1, uint32_t c2, bool p1, bool p2, uint32_t t)
	{
		if (p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
		if (p2)
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		qnet.add_gate(gate_kinds_t::mcx, std::vector<uint32_t>{{c1, c2}},
		              std::vector<uint32_t>{{t}});
		if (p2)
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		if (p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
	}

	void compute_or(uint32_t c1, uint32_t c2, bool p1, bool p2, uint32_t t)
	{
		if (!p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
		if (!p2)
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		qnet.add_gate(gate_kinds_t::mcx, std::vector<uint32_t>{{c1, c2}},
		              std::vector<uint32_t>{{t}});
		qnet.add_gate(gate_kinds_t::pauli_x, t);
		if (!p2)
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		if (!p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
	}

	void compute_xor(uint32_t c1, uint32_t c2, bool inv, uint32_t t)
	{
		qnet.add_gate(gate_kinds_t::cx, c1, t);
		qnet.add_gate(gate_kinds_t::cx, c2, t);
		if (inv)
			qnet.add_gate(gate_kinds_t::pauli_x, t);
	}

	void compute_xor3(uint32_t c1, uint32_t c2, uint32_t c3, bool inv, uint32_t t)
	{
		qnet.add_gate(gate_kinds_t::cx, c1, t);
		qnet.add_gate(gate_kinds_t::cx, c2, t);
		qnet.add_gate(gate_kinds_t::cx, c3, t);
		if (inv)
			qnet.add_gate(gate_kinds_t::pauli_x, t);
	}

	void compute_maj(uint32_t c1, uint32_t c2, uint32_t c3, bool p1, bool p2, bool p3, uint32_t t)
	{
		if (p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
		if (!p2) /* control 2 behaves opposite */
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		if (p3)
			qnet.add_gate(gate_kinds_t::pauli_x, c3);
		qnet.add_gate(gate_kinds_t::cx, c1, c2);
		qnet.add_gate(gate_kinds_t::cx, c3, c1);
		qnet.add_gate(gate_kinds_t::cx, c3, t);
		qnet.add_gate(gate_kinds_t::mcx, std::vector<uint32_t>{{c1, c2}},
		              std::vector<uint32_t>{{t}});
		qnet.add_gate(gate_kinds_t::cx, c3, c1);
		qnet.add_gate(gate_kinds_t::cx, c1, c2);
		if (p3)
			qnet.add_gate(gate_kinds_t::pauli_x, c3);
		if (!p2)
			qnet.add_gate(gate_kinds_t::pauli_x, c2);
		if (p1)
			qnet.add_gate(gate_kinds_t::pauli_x, c1);
	}

	void compute_lut(kitty::dynamic_truth_table const& function,
	                 std::vector<uint32_t> const& controls, uint32_t t)
	{
		auto qubit_map = controls;
		qubit_map.push_back(t);
		stg_from_pprm()(qnet, function, qubit_map);
	}

private:
	QuantumNetwork& qnet;
	LogicNetwork const& ntk;
	logic_network_synthesis_params const& ps;
	mt::node_map<uint32_t, LogicNetwork> node_to_qubit;
	std::stack<uint32_t> free_ancillae;
};

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
	static_assert(mt::is_network_type_v<LogicNetwork>, "LogicNetwork is not a network type");

	detail::logic_network_synthesis_impl<QuantumNetwork, LogicNetwork, MappingStrategy> impl(qnet,
	                                                                                         ntk,
	                                                                                         ps);
	impl.run();
}

} /* namespace tweedledum */
