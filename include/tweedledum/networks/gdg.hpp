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

#pragma region Add single-qubit gates
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
