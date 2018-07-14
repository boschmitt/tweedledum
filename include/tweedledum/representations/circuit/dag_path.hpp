/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt < bruno [at] oschmitt [dot] com >
*-----------------------------------------------------------------------------*/
#pragma once

#include "foreach.hpp"

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
struct dag_path {
public:
	using gate_type = Gate;

	struct node_pointer {
	public:
		static constexpr auto max
		    = std::numeric_limits<std::uint32_t>::max();

		node_pointer() = default;

		node_pointer(std::uint32_t data)
		    : data(data)
		{}

		node_pointer(std::uint32_t index, std::uint32_t flag)
		    : flag(flag)
		    , index(index)
		{}

		union {
			std::uint32_t data;
			struct {
				std::uint32_t flag : 1;
				std::uint32_t index : 31;
			};
		};

		bool operator==(node_pointer const& other) const
		{
			return data == other.data;
		}
	};

	struct node {
		gate_type gate;
		std::array<std::vector<node_pointer>, gate_type::max_num_qubits>
		    qubit;

		bool is_input() const
		{
			if (qubit[0].empty() && qubit[1].empty()) {
				return true;
			}
			return false;
		}

		bool is_output() const
		{
			if (qubit[0].empty() && not qubit[1].empty()) {
				return true;
			}
			return false;
		}
	};

public:
	dag_path()
	{
		nodes.reserve(1024u);
	}

	std::uint32_t create_qubit()
	{
		// std::cout << "Create qubit\n";
		auto qubit_id = static_cast<std::uint32_t>(inputs.size());
		auto index = nodes.size();

		// Create input node
		auto& input_node = nodes.emplace_back();
		input_node.gate.target(qubit_id);
		inputs.emplace_back(index, false);

		// Create ouput node
		auto& output_node = outputs.emplace_back();
		output_node.qubit[1].emplace_back(index, true);
		output_node.gate.target(qubit_id);

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
	void foreach_child(node& n, Fn&& fn) const
	{
		static_assert(detail::is_callable_without_index_v<Fn, node_pointer, void> ||
		              detail::is_callable_with_index_v<Fn, node_pointer, void> );

		for (auto qubit_id = 0u; qubit_id < n.qubit.size(); ++qubit_id) {
			auto begin = n.qubit[qubit_id].begin();
			auto end = n.qubit[qubit_id].end();
			while (begin != end) {
				if constexpr (detail::is_callable_without_index_v<Fn, node_pointer, void>) {
					fn(*begin++);
				} else if constexpr (detail::is_callable_with_index_v<Fn, node_pointer, void>) {
					fn(*begin++, qubit_id);
				}
			}
		}
	}

	template<typename Fn>
	void foreach_child(node& n, std::uint32_t qubit_id, Fn&& fn) const
	{
		auto index = n.gate.get_input_id(qubit_id);
		for (auto arc : n.qubit[index]) {
			if (arc.flag) {
				fn(arc);
			}
		}
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
			node.qubit[input_id].emplace_back(arc);
		});
		output.qubit[1].clear();
		output.qubit[1].emplace_back(node_index, true);
		return;
	}

public:
	std::vector<node_pointer> inputs;
	std::vector<node> nodes;
	std::vector<node> outputs;
};

} // namespace tweedledum
