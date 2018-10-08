/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"
#include "../utils/detail/foreach.hpp"
#include "detail/storage.hpp"

#include <array>
#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

namespace tweedledum {

/*! Directed acyclic graph (dag) path representation.
 *
 * Represent a quantum circuit as a directed acyclic graph. The nodes in the
 * graph are either input/output nodes or operation nodes. All nodes store
 * a gate object, which is defined as a class template parameter, which allows
 * great flexibility in the types supported as gates.
 *
 * Path DAG: the edges encode only the input/output relationship between the
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
	using node_type = uniform_node<gate_type, 1, 1>;
	using node_ptr_type = typename node_type::pointer_type;
	using storage_type = storage<node_type>;

	gg_network()
	    : storage_(std::make_shared<storage_type>())
	{}

	gg_network(std::shared_ptr<storage_type> storage)
	    : storage_(storage)
	{}
#pragma endregion

#pragma region I / O and ancillae qubits
private:
	auto create_qubit()
	{
		// std::cout << "Create qubit\n";
		auto qubit_id = static_cast<uint32_t>(storage_->inputs.size());
		auto index = static_cast<uint32_t>(storage_->nodes.size());

		// Create input node
		storage_->nodes.emplace_back(gate_kinds_t::input, qubit_id);
		storage_->inputs.emplace_back(index, false);

		// Create ouput node
		auto& output_node = storage_->outputs.emplace_back(gate_kinds_t::output, qubit_id);
		output_node.qubit[0][0] = {index, true};

		// std::cout << "[done]\n";
		return qubit_id;
	}

public:
	/* TODO: Remove */
	auto allocate_qubit()
	{
		auto str = fmt::format("q{}", storage_->inputs.size());
		return add_qubit(str);
	}

	auto add_qubit()
	{
		auto str = fmt::format("q{}", storage_->inputs.size());
		return add_qubit(str);
	}

	auto add_qubit(std::string const& qubit)
	{
		auto qubit_id = create_qubit();
		storage_->label_to_id.emplace(qubit, qubit_id);
		storage_->id_to_label.emplace_back(qubit);
		return qubit_id;
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
		if (node.gate.is(gate_kinds_t::output)) {
			auto index = &node - storage_->outputs.data();
			return static_cast<uint32_t>(index + storage_->nodes.size());
		}
		return static_cast<uint32_t>(&node - storage_->nodes.data());
	}

	auto get_children(node_type const& node, uint32_t qubit_id)
	{
		std::vector<node_ptr_type> ret;
		auto input_id = node.gate.qubit_index(qubit_id);
		auto child = node.qubit[input_id][0];
		auto child_node = get_node(child);
		while (not node.gate.is_dependent(child_node.gate)) {
			ret.emplace_back(child);
			input_id = child_node.gate.qubit_index(qubit_id);
			child = child_node.qubit[input_id][0];
			child_node = get_node(child);
		}
		ret.emplace_back(child);
		return ret;
	}

	void get_predecessor_choices(node_type const& node, std::vector<node_ptr_type>& choices)
	{
		if (node.gate.is(gate_kinds_t::input)) {
			return;
		}
		mark(node, 1);
		node.gate.foreach_control([&](auto qubit_id) {
			auto child = node.qubit[qubit_id][0];
			while (1) {
				auto const& child_node = get_node(child);
				if (node.gate.is_dependent(child_node.gate)) {
					break;
				}
				mark(child_node, 1);
				choices.emplace_back(child);
				qubit_id = child_node.gate.qubit_index(qubit_id);
				child = child_node.qubit[qubit_id][0];
			}
			mark(get_node(choices.back()), 0);
		});

		node.gate.foreach_target([&](auto qubit_id) {
			auto child = node.qubit[qubit_id][0];
			while (1) {
				auto const& child_node = get_node(child);
				if (node.gate.is_dependent(child_node.gate)) {
					break;
				}
				mark(child_node, 1);
				choices.emplace_back(child);
				qubit_id = child_node.gate.qubit_index(qubit_id);
				child = child_node.qubit[qubit_id][0];
			}
			mark(get_node(choices.back()), 0);
		});
		// for (auto aaa : choices) {
		// 	if (mark(get_node(aaa)))
		// 		std::cout << "Choi1: \n";
		// }
		// std::cout << "Choices: " << choices.size() << "\n\n";
	}

	auto get_predecessor_choices(node_type const& node)
	{
		std::vector<node_ptr_type> choices;
		// std::cout << "Node: " << node_to_index(node) << "\n";
		// std::cout << "Node: " << gate_name(node.gate.kind()) << "\n";

		node.gate.foreach_control([&](auto qubit_id) {
			auto child = node.qubit[qubit_id][0];
			auto const& child_node = get_node(child);
			choices.emplace_back(child);
			get_predecessor_choices(child_node, choices);
		});
		node.gate.foreach_target([&](auto qubit_id) {
			auto child = node.qubit[qubit_id][0];
			auto const& child_node = get_node(child);
			choices.emplace_back(child);
			get_predecessor_choices(child_node, choices);
		});
		return choices;
	}
#pragma endregion

#pragma region Add gates
private:
	void connect_node(uint32_t qubit_id, uint32_t node_index)
	{
		assert(storage_->outputs[qubit_id].qubit[1].size() > 0);
		auto& node = storage_->nodes.at(node_index);
		auto& output = storage_->outputs.at(qubit_id);

		// std::cout << "[" << qubit_id << ":" << node_index
		//           << "] Connecting node ("
		//           << gate_name(node.gate.kind()) << ")\n";

		auto connector = node.gate.qubit_index(qubit_id);
		foreach_child(output,
		              [&node, connector](auto arc) { node.qubit[connector][0] = arc; });
		output.qubit[qubit_id][0] = {node_index, true};
		return;
	}

	auto& do_add_gate(gate_type gate)
	{
		auto node_index = storage_->nodes.size();
		auto& node = storage_->nodes.emplace_back(gate);
		mark(node, default_mark_);

		node.gate.foreach_control([&](auto qubit_id) { connect_node(qubit_id, node_index); });
		node.gate.foreach_target([&](auto qubit_id) { connect_node(qubit_id, node_index); });
		return node;
	}

public:
	auto& add_gate(gate_type g)
	{
		return do_add_gate(g);
	}

	auto& add_gate(gate_kinds_t kind, uint32_t target, float rotation_angle = 0.0)
	{
		gate_type gate(kind, target, rotation_angle);
		return add_gate(gate);
	}

	auto& add_gate(gate_kinds_t kind, uint32_t control, uint32_t target, float rotation_angle = 0.0)
	{
		gate_type gate(kind, control, target, rotation_angle);
		return add_gate(gate);
	}

	auto& add_gate(gate_kinds_t kind, std::vector<uint32_t> const& controls,
	               std::vector<uint32_t> const& targets, float rotation_angle = 0.0)
	{
		gate_type gate(kind, controls, targets, rotation_angle);
		return add_gate(gate);
	}

	auto& add_gate(gate_kinds_t kind, std::string const& target, float rotation_angle = 0.0)
	{
		auto qubit_id = storage_->label_to_id[target];
		return add_gate(kind, qubit_id, rotation_angle);
	}

	auto& add_gate(gate_kinds_t kind, std::vector<std::string> const& controls,
	               std::vector<std::string> const& targets, float rotation_angle = 0.0)
	{
		std::vector<uint32_t> controls_id;
		std::vector<uint32_t> targets_id;
		for (auto& control : controls) {
			controls_id.push_back(storage_->label_to_id[control]);
		}
		for (auto& target : targets) {
			targets_id.push_back(storage_->label_to_id[target]);
		}
		return add_gate(kind, controls_id, targets_id, rotation_angle);
	}

	// Maybe remove it
	auto& add_gate(gate_kinds_t kind, std::string const& control, std::string const& target,
	               float rotation_angle = 0.0)
	{
		std::vector<uint32_t> control_id = {storage_->label_to_id[control]};
		std::vector<uint32_t> target_id = {storage_->label_to_id[target]};
		return add_gate(kind, control_id, target_id, rotation_angle);
	}
#pragma endregion

#pragma region Const node iterators
	template<typename Fn>
	void foreach_qubit(Fn&& fn) const
	{
		auto index = 0u;
		for (auto& label : storage_->id_to_label) {
			fn(index++, label);
		}
	}

	template<typename Fn>
	void foreach_input(Fn&& fn) const
	{
		for (auto arc : storage_->inputs) {
			fn(storage_->nodes[arc.index], arc.index);
		}
	}

	template<typename Fn>
	void foreach_output(Fn&& fn) const
	{
		uint32_t index = storage_->nodes.size();
		detail::foreach_element(storage_->outputs.cbegin(), storage_->outputs.cend(), fn,
		                        index);
	}

	template<typename Fn>
	void foreach_node(Fn&& fn) const
	{
		detail::foreach_element(storage_->nodes.cbegin(), storage_->nodes.cend(), fn);
		detail::foreach_element(storage_->outputs.cbegin(), storage_->outputs.cend(), fn,
		                        storage_->nodes.size());
	}

	template<typename Fn>
	void foreach_gate(Fn&& fn) const
	{
		uint32_t index = storage_->inputs.size();
		auto begin = storage_->nodes.cbegin() + index;
		auto end = storage_->nodes.cend();
		detail::foreach_element(begin, end, fn, index);
	}

	template<typename Fn>
	void foreach_child(node_type const& n, Fn&& fn) const
	{
		static_assert(detail::is_callable_without_index_v<
		                  Fn, node_ptr_type,
		                  void> || detail::is_callable_with_index_v<Fn, node_ptr_type, void>);

		for (auto qubit_id = 0u; qubit_id < n.qubit.size(); ++qubit_id) {
			if (n.qubit[qubit_id][0] == node_ptr_type::max) {
				continue;
			}
			if constexpr (detail::is_callable_without_index_v<Fn, node_ptr_type, void>) {
				fn(n.qubit[qubit_id][0]);
			} else if constexpr (detail::is_callable_with_index_v<Fn, node_ptr_type, void>) {
				fn(n.qubit[qubit_id][0], qubit_id);
			}
		}
	}

	template<typename Fn>
	void foreach_child(node_type const& n, uint32_t qubit_id, Fn&& fn) const
	{
		auto index = n.gate.qubit_index(qubit_id);
		if (n.qubit[index][0] == node_ptr_type::max) {
			return;
		}
		fn(n.qubit[index][0]);
	}
#pragma endregion

#pragma region Visited flags
	void clear_marks()
	{
		std::for_each(storage_->nodes.begin(), storage_->nodes.end(),
		              [](auto& n) { n.data[0].b0 = 0; });
		std::for_each(storage_->outputs.begin(), storage_->outputs.end(),
		              [](auto& n) { n.data[0].b0 = 0; });
	}

	auto mark(node_type const& n) const
	{
		return n.data[0].b0;
	}

	void mark(node_type const& n, std::uint8_t value)
	{
		n.data[0].b0 = value;
	}

	void default_mark(std::uint8_t value)
	{
		default_mark_ = value;
	}
#pragma endregion

private:
	std::shared_ptr<storage_type> storage_;
	uint8_t default_mark_{0};
};

} // namespace tweedledum
