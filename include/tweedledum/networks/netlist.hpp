/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"
#include "../utils/detail/foreach.hpp"
#include "detail/storage.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <vector>

namespace tweedledum {

template<typename GateType>
class netlist {
public:
#pragma region Types and constructors
	using gate_type = GateType;
	using node_type = wrapper_node<gate_type, 1>;
	using node_ptr_type = typename node_type::pointer_type;
	using storage_type = storage<node_type>;

	netlist()
	    : storage_(std::make_shared<storage_type>())
	{}

	netlist(std::shared_ptr<storage_type> storage)
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
		auto& input_node = storage_->nodes.emplace_back(gate_kinds_t::input, qubit_id);
		storage_->inputs.emplace_back(index);
		mark(input_node, default_mark_);

		// Create ouput node
		auto& output_node = storage_->outputs.emplace_back(gate_kinds_t::output, qubit_id);
		mark(output_node, default_mark_);

		// std::cout << "[done]\n";
		return qubit_id;
	}

public:
	auto add_qubit()
	{
		auto str = fmt::format("q{}", storage_->inputs.size());
		return add_qubit(str);
	}

	auto add_qubit(std::string const& label)
	{
		auto qubit_id = create_qubit();
		storage_->label_to_id.emplace(label, qubit_id);
		storage_->id_to_label.emplace_back(label);
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
		// if (node_ptr.index >= storage_->nodes.size()) {
		// 	return storage_->outputs[node_ptr.index - storage_->nodes.size()];
		// }
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
#pragma endregion

#pragma region Add gates
	auto& add_gate(gate_type g)
	{
		auto& node = storage_->nodes.emplace_back(g);
		return node;
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

	void mark(node_type const& n, uint8_t value)
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
	uint32_t default_mark_ = 0u;
};

} // namespace tweedledum
