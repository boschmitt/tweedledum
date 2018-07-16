/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include "../quantum_circuit_interface.hpp"
#include "foreach.hpp"
#include "nodes.hpp"

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
template<typename Gate>
struct dag_path final : public quantum_circuit_interface<Gate> {
public:
	using gate_type = Gate;
	using node_type = uniform_node<gate_type, gate_type::max_num_qubits, 1>;
	using node_ptr_type = typename node_type::pointer_type;

public:
	dag_path()
	{
		nodes.reserve(1024u);
	}

	std::uint32_t create_qubit()
	{
		// std::cout << "Create qubit\n";
		auto qubit_id = static_cast<std::uint32_t>(inputs.size());
		auto index = static_cast<std::uint32_t>(nodes.size());

		// Create input node
		auto& input_node = nodes.emplace_back();
		input_node.gate.kind(gate_kinds::input);
		input_node.gate.target(qubit_id);
		inputs.emplace_back(index, false);

		// Create ouput node
		auto& output_node = outputs.emplace_back();
		output_node.gate.kind(gate_kinds::output);
		output_node.gate.target(qubit_id);
		output_node.qubit[0][0] = {index, true};

		// std::cout << "[done]\n";
		return qubit_id;
	}

	void do_add_gate(gate_type gate)
	{
		auto node_index = nodes.size();
		auto& node = nodes.emplace_back();

		node.gate = gate;
		node.gate.foreach_control([&](auto qubit_id, auto control_id) {
			connect_node(qubit_id, control_id, node_index);
		});
		node.gate.foreach_target([&](auto qubit_id, auto target_id) {
			connect_node(qubit_id, target_id, node_index);
		});
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
		detail::foreach_element(nodes.begin(), nodes.end(), fn);
		detail::foreach_element(outputs.begin(), outputs.end(), fn, nodes.size());
	}

	template<typename Fn>
	void foreach_input(Fn&& fn) const
	{
		for (auto arc : inputs) {
			fn(nodes[arc.index], arc.index);
		}
	}

	template<typename Fn>
	void foreach_output(Fn&& fn) const
	{
		std::uint32_t index = nodes.size();
		detail::foreach_element(outputs.begin(), outputs.end(), fn, index);
	}

	template<typename Fn>
	void foreach_gate(Fn&& fn) const
	{
		std::uint32_t index = inputs.size();
		auto begin = nodes.begin() + index;
		auto end = nodes.end();
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

private:
	void connect_node(std::uint32_t qubit_id, std::uint32_t input_id,
	                  std::uint32_t node_index)
	{
		assert(outputs[qubit_id].qubit[1].size() > 0);
		auto& node = nodes.at(node_index);
		auto& output = outputs.at(qubit_id);

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

public:
	std::vector<node_ptr_type> inputs;
	std::vector<node_type> nodes;
	std::vector<node_type> outputs;
};

} // namespace tweedledum
