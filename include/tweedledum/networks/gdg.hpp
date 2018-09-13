/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "detail/foreach.hpp"
#include "detail/storage.hpp"
#include "gates/gate_kinds.hpp"

#include <fmt/format.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace tweedledum {

/*! Gate dependecy graph
 *
 * Represent a quantum circuit as a directed acyclic graph. The nodes in the
 * graph are either input/output nodes or operation nodes. All nodes store
 * a gate object, which is defined as a class template parameter, which allows
 * great flexibility in the types supported as gates.
 *
 */
template<typename GateType>
struct gdg {
public:
#pragma region Types and constructors
	using gate_type = GateType;
	using node_type = regular_node<gate_type, 1, 1>;
	using node_ptr_type = typename node_type::pointer_type;
	using storage_type = storage<node_type>;

	gdg()
	    : storage_(std::make_shared<storage_type>())
	{}

	gdg(std::shared_ptr<storage_type> storage)
	    : storage_(storage)
	{}
#pragma endregion

#pragma region I/O and ancillae qubits
private:
	auto create_qubit()
	{
		// std::cout << "Create qubit\n";
		auto qubit_id = static_cast<uint32_t>(storage_->inputs.size());
		auto index = static_cast<uint32_t>(storage_->nodes.size());

		// Create input node
		auto& input_node = storage_->nodes.emplace_back();
		input_node.gate.kind(gate_kinds_t::input);
		input_node.gate.target_qubit(qubit_id);
		storage_->inputs.emplace_back(index, false);

		// Create ouput node
		auto& output_node = storage_->outputs.emplace_back();
		output_node.gate.kind(gate_kinds_t::output);
		output_node.gate.target_qubit(qubit_id);
		output_node.qubit[0].emplace_back(index, true);

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
		label_to_id_.emplace(label, qubit_id);
		id_to_label_.emplace_back(label);
		return qubit_id;
	}
#pragma endregion

#pragma region Structural properties
	auto size() const
	{
		return static_cast<uint32_t>(storage_->nodes.size()
		                             + storage_->outputs.size());
	}

	auto num_gates() const
	{
		return static_cast<uint32_t>(storage_->nodes.size()
		                             - storage_->inputs.size());
	}

	auto num_qubits() const
	{
		return static_cast<uint32_t>(storage_->inputs.size());
	}
#pragma endregion

#pragma region Add gates
private:
	void connect_node(std::uint32_t qubit_id, std::uint32_t node_index)
	{
		auto& node = storage_->nodes.at(node_index);
		auto& output = storage_->outputs.at(qubit_id);
		auto previous_node_arc = output.qubit[0].back();
		auto& previous_node = storage_->nodes.at(previous_node_arc.index);
		auto connector = node.gate.get_input_id(qubit_id);
		if (node.gate.is_dependent(previous_node.gate)) {
			foreach_child(output, [&node, connector](auto arc) {
				node.qubit[connector].emplace_back(arc);
			});
			output.qubit[0].clear();
			output.qubit[0].emplace_back(node_index, true);
			return;
		}
		output.qubit[0].emplace_back(node_index, true);
		foreach_child(previous_node, qubit_id, [&node, connector](auto arc) {
			node.qubit[connector].emplace_back(arc);
		});
	}

	node_type& do_add_gate(gate_type gate)
	{
		auto node_index = storage_->nodes.size();
		auto& node = storage_->nodes.emplace_back();
		// mark(node, default_mark_);

		node.gate = gate;
		node.gate.foreach_control(
		    [&](auto qubit_id) { connect_node(qubit_id, node_index); });
		node.gate.foreach_target(
		    [&](auto qubit_id) { connect_node(qubit_id, node_index); });
		return node;
	}

public:
	auto& add_gate(gate_type g)
	{
		return do_add_gate(g);
	}

	auto& add_gate(gate_kinds_t kind, std::string const& target)
	{
		auto qubit_id = label_to_id_[target];
		return add_gate(kind, qubit_id);
	}

	auto& add_gate(gate_kinds_t kind, uint32_t target_id)
	{
		gate_type single_target_gate(kind, target_id);
		return do_add_gate(single_target_gate);
	}

	auto& add_x_rotation(std::string const& label, float angle)
	{
		auto qubit_id = label_to_id_[label];
		return add_x_rotation(qubit_id, angle);
	}

	auto& add_z_rotation(std::string const& label, float angle)
	{
		auto qubit_id = label_to_id_[label];
		return add_z_rotation(qubit_id, angle);
	}

	auto& add_x_rotation(uint32_t target_id, float angle)
	{
		gate_type rotation_gate(gate_kinds_t::rotation_x, target_id,
		                        angle);
		return do_add_gate(rotation_gate);
	}

	auto& add_z_rotation(uint32_t target_id, float angle)
	{
		gate_type rotation_gate(gate_kinds_t::rotation_z, target_id,
		                        angle);
		return do_add_gate(rotation_gate);
	}

	auto& add_controlled_gate(gate_kinds_t kind, std::string const& control,
	                          std::string const& target)
	{
		auto target_id = label_to_id_[target];
		auto control_id = label_to_id_[control];
		return add_controlled_gate(kind, control_id, target_id);
	}

	auto& add_controlled_gate(gate_kinds_t kind, uint32_t control_id,
	                          uint32_t target_id)
	{
		gate_type controlled_gate(kind, target_id, control_id);
		return do_add_gate(controlled_gate);
	}

	auto& add_multiple_controlled_gate(gate_kinds_t kind,
	                                   std::vector<std::string> const& labels)
	{
		std::vector<uint32_t> qubits;
		for (auto& label : labels) {
			qubits.push_back(label_to_id_[label]);
		}
		return add_multiple_controlled_gate(kind, qubits);
	}

	auto& add_multiple_controlled_gate(gate_kinds_t kind,
	                                   std::vector<uint32_t> const& qubits)
	{
		gate_type multiple_controlled_gate(kind, qubits[0], qubits[1],
		                                   qubits[2]);
		return do_add_gate(multiple_controlled_gate);
	}
#pragma endregion

#pragma region Const node iterators
	template<typename Fn>
	void foreach_qubit(Fn&& fn) const
	{
		auto index = 0u;
		for (auto& label : id_to_label_) {
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
		detail::foreach_element(storage_->outputs.cbegin(),
		                        storage_->outputs.cend(), fn, index);
	}

	template<typename Fn>
	void foreach_node(Fn&& fn) const
	{
		detail::foreach_element(storage_->nodes.cbegin(),
		                        storage_->nodes.cend(), fn);
		detail::foreach_element(storage_->outputs.cbegin(),
		                        storage_->outputs.cend(), fn,
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
	void foreach_child(node_type& n, Fn&& fn) const
	{
		static_assert(
		    detail::is_callable_without_index_v<
		        Fn, node_ptr_type,
		        void> || detail::is_callable_with_index_v<Fn, node_ptr_type, void>);

		for (auto qubit_id = 0u; qubit_id < n.qubit.size(); ++qubit_id) {
			auto begin = n.qubit[qubit_id].begin();
			auto end = n.qubit[qubit_id].end();
			while (begin != end) {
				if constexpr (detail::is_callable_without_index_v<
						Fn, node_ptr_type, void>) {
					fn(*begin++);
				} else if constexpr (detail::is_callable_with_index_v<
							Fn, node_ptr_type, void>) {
					fn(*begin++, qubit_id);
				}
			}
		}
	}

	template<typename Fn>
	void foreach_child(node_type& n, uint32_t qubit_id, Fn&& fn) const
	{
		auto index = n.gate.get_input_id(qubit_id);
		for (auto arc : n.qubit[index]) {
			fn(arc);
		}
	}
#pragma endregion

#pragma region Node iterators
	template<typename Fn>
	void foreach_qubit(Fn&& fn)
	{
		auto index = 0u;
		for (auto& label : id_to_label_) {
			fn(index++, label);
		}
	}

	template<typename Fn>
	void foreach_input(Fn&& fn)
	{
		for (auto arc : storage_->inputs) {
			fn(storage_->nodes[arc.index], arc.index);
		}
	}

	template<typename Fn>
	void foreach_output(Fn&& fn)
	{
		uint32_t index = storage_->nodes.size();
		detail::foreach_element(storage_->outputs.cbegin(),
		                        storage_->outputs.cend(), fn, index);
	}

	template<typename Fn>
	void foreach_node(Fn&& fn)
	{
		detail::foreach_element(storage_->nodes.cbegin(),
		                        storage_->nodes.cend(), fn);
		detail::foreach_element(storage_->outputs.cbegin(),
		                        storage_->outputs.cend(), fn,
		                        storage_->nodes.size());
	}

	template<typename Fn>
	void foreach_gate(Fn&& fn)
	{
		uint32_t index = storage_->inputs.size();
		auto begin = storage_->nodes.cbegin() + index;
		auto end = storage_->nodes.cend();
		detail::foreach_element(begin, end, fn, index);
	}
	
	template<typename Fn>
	void foreach_child(node_type& n, Fn&& fn)
	{
		static_assert(
		    detail::is_callable_without_index_v<
		        Fn, node_ptr_type,
		        void> || detail::is_callable_with_index_v<Fn, node_ptr_type, void>);

		for (auto qubit_id = 0u; qubit_id < n.qubit.size(); ++qubit_id) {
			auto begin = n.qubit[qubit_id].begin();
			auto end = n.qubit[qubit_id].end();
			while (begin != end) {
				if constexpr (detail::is_callable_without_index_v<
						Fn, node_ptr_type, void>) {
					fn(*begin++);
				} else if constexpr (detail::is_callable_with_index_v<
							Fn, node_ptr_type, void>) {
					fn(*begin++, qubit_id);
				}
			}
		}
	}

	template<typename Fn>
	void foreach_child(node_type& n, uint32_t qubit_id, Fn&& fn)
	{
		auto index = n.gate.get_input_id(qubit_id);
		for (auto arc : n.qubit[index]) {
			fn(arc);
		}
	}
#pragma endregion

#pragma region Visited flags
#pragma endregion

private:
	std::unordered_map<std::string, uint32_t> label_to_id_;
	std::vector<std::string> id_to_label_;
	std::shared_ptr<storage_type> storage_;
};

} // namespace tweedledum
