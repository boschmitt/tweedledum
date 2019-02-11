/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_base.hpp"
#include "../utils/foreach.hpp"
#include "detail/storage.hpp"
#include "qubit.hpp"

#include <array>
#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

namespace tweedledum {

/*! Gate Graph (GG) is a directed acyclic graph (DAG) representation.
 *
 * Represent a quantum circuit as a directed acyclic graph. The nodes in the
 * graph are either input/output nodes or operation nodes. All nodes store
 * a gate object, which is defined as a class template parameter, which allows
 * great flexibility in the types supported as gates.
 *
 * The edges encode only the input/output relationship between the
 * gates. That is, a directed edge from node A to node B means that the qubit
 * _must_ pass from the output of A to the input of B.
 *
 * Some natural properties like depth can be computed directly from the graph.
 */
template<typename GateType>
class gg_network {
public:
#pragma region Types and constructors
	using gate_type = GateType;
	using node_type = uniform_node<gate_type, 0, 1>;
	using node_ptr_type = typename node_type::pointer_type;
	using storage_type = storage<node_type>;

	gg_network()
	    : storage_(std::make_shared<storage_type>())
	    , qlabels_(std::make_shared<qlabels_map>())
	{}
#pragma endregion

#pragma region I / O and ancillae qubits
private:
	auto create_qubit()
	{
		qubit_id qid(storage_->inputs.size());
		uint32_t index = storage_->nodes.size();
		gate_type input(gate_base(gate_set::input), qid);
		gate_type output(gate_base(gate_set::output), qid);

		storage_->nodes.emplace_back(input);
		storage_->inputs.emplace_back(index);

		auto& output_node = storage_->outputs.emplace_back(output);
		output_node.children[0] = index;

		storage_->rewiring_map.push_back(qid);
		return qid;
	}

public:
	auto add_qubit(std::string const& qlabel)
	{
		auto qid = create_qubit();
		qlabels_->map(qid, qlabel);
		return qid;
	}

	auto add_qubit()
	{
		auto qlabel = fmt::format("q{}", storage_->inputs.size());
		return add_qubit(qlabel);
	}
#pragma endregion

#pragma region Structural properties
	auto size() const
	{
		return static_cast<uint32_t>(storage_->nodes.size() + storage_->outputs.size());
	}

	auto num_qubits() const
	{
		return static_cast<uint32_t>(storage_->inputs.size());
	}

	auto num_gates() const
	{
		return static_cast<uint32_t>(storage_->nodes.size() - storage_->inputs.size());
	}
#pragma endregion

#pragma region Nodes
	auto& get_node(node_ptr_type node_ptr) const
	{
		return storage_->nodes[node_ptr.index];
	}

	auto node_to_index(node_type const& node) const
	{
		if (node.gate.is(gate_set::output)) {
			auto index = &node - storage_->outputs.data();
			return static_cast<uint32_t>(index + storage_->nodes.size());
		}
		return static_cast<uint32_t>(&node - storage_->nodes.data());
	}
#pragma endregion

#pragma region Add gates(qid)
private:
	void connect_node(qubit_id qid, uint32_t node_index)
	{
		auto& node = storage_->nodes.at(node_index);
		auto& output = storage_->outputs.at(qid.index());
		auto slot = node.gate.qubit_slot(qid);

		assert(output.children[0].data != node_ptr_type::max);
		foreach_child(output, [&node, slot](auto arc) {
			node.children[slot] = arc;
		});
		output.children[0] = node_index;
		return;
	}

	auto& do_add_gate(gate_type gate)
	{
		auto node_index = storage_->nodes.size();
		auto& node = storage_->nodes.emplace_back(gate);

		node.gate.foreach_control([&](auto& qid) { connect_node(qid, node_index); });
		node.gate.foreach_target([&](auto& qid) { connect_node(qid, node_index); });
		return node;
	}

public:
	auto& add_gate(gate_type const& gate)
	{
		return do_add_gate(gate);
	}

	auto& add_gate(gate_base op, qubit_id target)
	{
		gate_type gate(op, storage_->rewiring_map.at(target));
		return add_gate(gate);
	}

	auto& add_gate(gate_base op, qubit_id control, qubit_id target)
	{
		gate_type gate(op,
		               qubit_id(storage_->rewiring_map.at(control), control.is_complemented()),
		               storage_->rewiring_map.at(target));
		return add_gate(gate);
	}

	auto& add_gate(gate_base op, std::vector<qubit_id> controls, std::vector<qubit_id> targets)
	{
		std::transform(controls.begin(), controls.end(), controls.begin(),
		               [&](qubit_id qid) -> qubit_id {
			               return qubit_id(storage_->rewiring_map.at(qid),
			                               qid.is_complemented());
		               });
		std::transform(targets.begin(), targets.end(), targets.begin(),
		               [&](qubit_id qid) -> qubit_id {
			               return storage_->rewiring_map.at(qid);
		               });
		gate_type gate(op, controls, targets);
		return add_gate(gate);
	}
#pragma endregion

#pragma region Add gates(qlabels)
	auto& add_gate(gate_base op, std::string const& qlabel_target)
	{
		auto qid_target = qlabels_->to_qid(qlabel_target);
		gate_type gate(op, qid_target);
		return add_gate(gate);
	}

	auto& add_gate(gate_base op, std::string const& qlabel_control,
	               std::string const& qlabel_target)
	{
		auto qid_control = qlabels_->to_qid(qlabel_control);
		auto qid_target = qlabels_->to_qid(qlabel_target);
		gate_type gate(op, qid_control, qid_target);
		return add_gate(gate);
	}

	auto& add_gate(gate_base op, std::vector<std::string> const& qlabels_control,
	               std::vector<std::string> const& qlabels_target)
	{
		std::vector<qubit_id> controls;
		for (auto& control : qlabels_control) {
			controls.push_back(qlabels_->to_qid(control));
		}
		std::vector<qubit_id> targets;
		for (auto& target : qlabels_target) {
			targets.push_back(qlabels_->to_qid(target));
		}
		gate_type gate(op, controls, targets);
		return add_gate(gate);
	}
#pragma endregion

#pragma region Const iterators
	template<typename Fn>
	qubit_id foreach_cqubit(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, qubit_id> ||
			      std::is_invocable_r_v<bool, Fn, qubit_id> ||
		              std::is_invocable_r_v<void, Fn, std::string const&> || 
			      std::is_invocable_r_v<void, Fn, qubit_id, std::string const&>);
		// clang-format on
		if constexpr (std::is_invocable_r_v<bool, Fn, qubit_id>) {
			for (auto qid = 0u; qid < num_qubits(); ++qid) {
				if (!fn(qubit_id(qid))) {
					return qid;
				}
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, qubit_id>) {
			for (auto qid = 0u; qid < num_qubits(); ++qid) {
				fn(qubit_id(qid));
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, std::string const&>) {
			for (auto const& qlabel : *qlabels_) {
				fn(qlabel);
			}
		} else {
			auto qid = 0u;
			for (auto const& qlabel : *qlabels_) {
				fn(qubit_id(qid++), qlabel);
			}
		}
		return qid_invalid;
	}

	template<typename Fn>
	void foreach_cinput(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&, uint32_t> ||
		              std::is_invocable_r_v<void, Fn, node_type const&>);
		// clang-format on
		for (auto node_index : storage_->inputs) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_type const&, uint32_t>) {
				fn(storage_->nodes[node_index], node_index);
			} else {
				fn(storage_->nodes[node_index]);
			}
		}
	}

	template<typename Fn>
	void foreach_coutput(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&, uint32_t> ||
		              std::is_invocable_r_v<void, Fn, node_type const&>);
		// clang-format on
		uint32_t node_index = storage_->nodes.size();
		for (auto const& node : storage_->outputs) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_type const&, uint32_t>) {
				fn(node, node_index++);
			} else {
				fn(node);
			}
		}
	}

	template<typename Fn>
	void foreach_cgate(Fn&& fn) const
	{
		foreach_element_if(storage_->nodes.cbegin(), storage_->nodes.cend(),
		                   [](auto const& node) { return node.gate.is_unitary_gate(); }, fn);
	}

	template<typename Fn>
	void foreach_cnode(Fn&& fn) const
	{
		foreach_element(storage_->nodes.cbegin(), storage_->nodes.cend(), fn);
		foreach_element(storage_->outputs.cbegin(), storage_->outputs.cend(), fn,
		                storage_->nodes.size());
	}
#pragma endregion

#pragma region Const node iterators
	template<typename Fn>
	void foreach_child(node_type const& node, Fn&& fn) const
	{
		static_assert(is_callable_without_index_v< Fn, node_ptr_type, void>
		|| is_callable_with_index_v<Fn, node_ptr_type, void>);

		for (auto i = 0u; i < node.children.size(); ++i) {
			if (node.children[i] == node_ptr_type::max) {
				continue;
			}
			if constexpr (is_callable_without_index_v<Fn, node_ptr_type, void>) {
				fn(node.children[i]);
			} else if constexpr (is_callable_with_index_v<Fn, node_ptr_type, void>) {
				fn(node.children[i], i);
			}
		}
	}
#pragma endregion

#pragma region Rewiring
	void rewire(std::vector<uint32_t> const& rewiring_map)
	{
		storage_->rewiring_map = rewiring_map;
	}

	void rewire(std::vector<std::pair<uint32_t, uint32_t>> const& transpositions)
	{
		for (auto&& [i, j] : transpositions) {
			std::swap(storage_->rewiring_map[i], storage_->rewiring_map[j]);
		}
	}

	constexpr auto rewire_map() const
	{
		return storage_->rewiring_map;
	}
#pragma endregion

#pragma region Visited flags
	void clear_visited()
	{
		std::for_each(storage_->nodes.begin(), storage_->nodes.end(),
		              [](auto& node) { node.data[0].w = 0; });
		std::for_each(storage_->outputs.begin(), storage_->outputs.end(),
		              [](auto& node) { node.data[0].w = 0; });
	}

	auto visited(node_type const& node) const
	{
		return node.data[0].w;
	}

	void set_visited(node_type const& node, uint32_t value)
	{
		node.data[0].w = value;
	}
#pragma endregion

private:
	std::shared_ptr<storage_type> storage_;
	std::shared_ptr<qlabels_map> qlabels_;
};

} // namespace tweedledum
