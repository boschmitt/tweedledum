/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include "../quantum_circuit_interface.hpp"
#include "foreach.hpp"
#include "storage.hpp"

#include <array>
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
struct dag_path final : public quantum_circuit_interface<GateType> {
public:
	using gate_type = GateType;
	using node_type = uniform_node<gate_type, 1, 1>;
	using node_ptr_type = typename node_type::pointer_type;
	using storage_type = storage<node_type>;

public:
	dag_path()
	    : storage_(std::make_shared<storage_type>())
	{}

	dag_path(std::shared_ptr<storage_type> storage)
	    : storage_(storage)
	{}


	std::uint32_t size() const
	{
		return storage_->nodes.size() + storage_->outputs.size();
	}

	std::uint32_t num_qubits() const
	{
		return storage_->inputs.size();
	}

	std::uint32_t size() const
	{
		return nodes.size() + outputs.size();
	}

	std::uint32_t num_qubits() const
	{
		return inputs.size();
	}

	template<typename Fn>
	void foreach_node(Fn&& fn) const
	{
		detail::foreach_element(storage_->nodes.cbegin(), storage_->nodes.cend(), fn);
		detail::foreach_element(storage_->outputs.cbegin(), storage_->outputs.cend(), fn, storage_->nodes.size());
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
		std::uint32_t index = storage_->nodes.size();
		detail::foreach_element(storage_->outputs.cbegin(), storage_->outputs.cend(), fn, index);
	}

	template<typename Fn>
	void foreach_gate(Fn&& fn) const
	{
		std::uint32_t index = storage_->inputs.size();
		auto begin = storage_->nodes.cbegin() + index;
		auto end = storage_->nodes.cend();
		detail::foreach_element(begin, end, fn, index);
	}

	template<typename Fn>
	void foreach_child(node_type& n, Fn&& fn) const
	{
		static_assert(detail::is_callable_without_index_v<Fn, node_ptr_type, void> ||
		              detail::is_callable_with_index_v<Fn, node_ptr_type, void> );

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
	void foreach_child(node_type& n, std::uint32_t qubit_id, Fn&& fn) const
	{
		auto index = n.gate.get_input_id(qubit_id);
		if (n.qubit[index][0] == node_ptr_type::max) {
			return;
		}
		fn(n.qubit[index][0]);
	}

	template<typename Fn>
	void foreach_node(Fn&& fn)
	{
		detail::foreach_element(storage_->nodes.begin(), storage_->nodes.end(), fn);
		detail::foreach_element(storage_->outputs.begin(), storage_->outputs.end(), fn, storage_->nodes.size());
	}

	template<typename Fn>
	void foreach_gate(Fn&& fn)
	{
		std::uint32_t index = storage_->inputs.size();
		auto begin = storage_->nodes.begin() + index;
		auto end = storage_->nodes.end();
		detail::foreach_element(begin, end, fn, index);
	}

	template<typename Fn>
	void foreach_child(node_type& n, Fn&& fn)
	{
		static_assert(detail::is_callable_without_index_v<Fn, node_ptr_type, void> ||
		              detail::is_callable_with_index_v<Fn, node_ptr_type, void> );

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
	void foreach_child(node_type& n, std::uint32_t qubit_id, Fn&& fn)
	{
		auto index = n.gate.get_input_id(qubit_id);
		if (n.qubit[index][0] == node_ptr_type::max) {
			return;
		}
		fn(n.qubit[index][0]);
	}

private:

	std::uint32_t create_qubit()
	{
		// std::cout << "Create qubit\n";
		auto qubit_id = static_cast<std::uint32_t>(storage_->inputs.size());
		auto index = static_cast<std::uint32_t>(storage_->nodes.size());

		// Create input node
		auto& input_node = storage_->nodes.emplace_back();
		input_node.gate.kind(gate_kinds::input);
		input_node.gate.target(qubit_id);
		storage_->inputs.emplace_back(index, false);

		// Create ouput node
		auto& output_node = storage_->outputs.emplace_back();
		output_node.gate.kind(gate_kinds::output);
		output_node.gate.target(qubit_id);
		output_node.qubit[0][0] = {index, true};

		// std::cout << "[done]\n";
		return qubit_id;
	}

	void do_add_gate(gate_type gate)
	{
		auto node_index = storage_->nodes.size();
		auto& node = storage_->nodes.emplace_back();

		node.gate = gate;
		node.gate.foreach_control([&](auto qubit_id, auto control_id) {
			connect_node(qubit_id, control_id, node_index);
		});
		node.gate.foreach_target([&](auto qubit_id, auto target_id) {
			connect_node(qubit_id, target_id, node_index);
		});
	}

	void connect_node(std::uint32_t qubit_id, std::uint32_t input_id,
	                  std::uint32_t node_index)
	{
		assert(storage_->outputs[qubit_id].qubit[1].size() > 0);
		auto& node = storage_->nodes.at(node_index);
		auto& output = storage_->outputs.at(qubit_id);

		std::cout << "[" << qubit_id << ":" << node_index
		          << "] Connecting node ("
		          << gate_name(node.gate.kind()) << ")\n";

		foreach_child(output, [&node, input_id](auto arc) {
			node.qubit[input_id][0] = arc;
		});
		// output.qubit[1].clear();
		output.qubit[0][0] = {node_index, true};
		return;
	}

private:
	std::shared_ptr<storage_type> storage_;
};

} // namespace tweedledum
