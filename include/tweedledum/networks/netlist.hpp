/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_base.hpp"
#include "../utils/foreach.hpp"
#include "detail/storage.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <string>
#include <type_traits>

namespace tweedledum {

/*! \brief 
 */
template<typename GateType>
class netlist {
public:
#pragma region Types and constructors
	using gate_type = GateType;
	using node_type = wrapper_node<gate_type, 1>;
	using storage_type = storage<node_type>;

	netlist()
	    : storage_(std::make_shared<storage_type>())
	    , qlabels_(std::make_shared<qlabels_map>())
	{}
#pragma endregion

#pragma region I / O and ancillae qubits
private:
	auto create_qubit()
	{
		uint32_t qid = storage_->inputs.size();
		uint32_t index = storage_->nodes.size();
		gate_type input(gate_base(gate_set::input), qid);
		gate_type output(gate_base(gate_set::output), qid);

		storage_->nodes.emplace_back(input);
		storage_->inputs.emplace_back(index);
		storage_->outputs.emplace_back(output);
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
	/*! \brief Returns the number of nodes. */
	auto size() const
	{
		return static_cast<uint32_t>(storage_->nodes.size() + storage_->outputs.size());
	}

	/*! \brief Returns the number of qubits. */
	auto num_qubits() const
	{
		return static_cast<uint32_t>(storage_->inputs.size());
	}

	/*! \brief Returns the number of gates, i.e., nodes that hold unitary operations */
	// Meta gates such as input and outputs are not considered
	auto num_gates() const
	{
		return static_cast<uint32_t>(storage_->nodes.size() - storage_->inputs.size());
	}
#pragma endregion

#pragma region Nodes
#pragma endregion

#pragma region Add gates(qids)
	/*! \brief Add a gate to the network. */
	// This assumes the gate have been properly rewired
	auto& add_gate(gate_type const& gate)
	{
		// assert(!gate.is_meta());
		auto& node = storage_->nodes.emplace_back(gate);
		return node;
	}

	/*! \brief Add a gate to the network. */
	auto& add_gate(gate_base op, uint32_t qid_target)
	{
		gate_type gate(op, storage_->rewiring_map[qid_target]);
		return add_gate(gate);
	}

	/*! \brief Add a gate to the network. */
	auto& add_gate(gate_base op, uint32_t qid_control, uint32_t qid_target)
	{
		gate_type gate(op, storage_->rewiring_map[qid_control],
		               storage_->rewiring_map[qid_target]);
		return add_gate(gate);
	}

	/*! \brief Add a gate to the network. */
	auto& add_gate(gate_base op, std::vector<uint32_t> const& qids_control,
	               std::vector<uint32_t> const& qids_target)
	{
		std::vector<uint32_t> controls;
		std::transform(qids_control.begin(), qids_control.end(),
		               std::back_inserter(controls), [&](uint32_t qid) -> uint32_t {
			               return storage_->rewiring_map[qid];
		               });
		std::vector<uint32_t> targets;
		std::transform(qids_target.begin(), qids_target.end(),
		               std::back_inserter(targets), [&](uint32_t qid) -> uint32_t {
			               return storage_->rewiring_map[qid];
		               });
		gate_type gate(op, controls, targets);
		return add_gate(gate);
	}
#pragma endregion

#pragma region Add gates(qlabels)
	/*! \brief Add a gate to the network. */
	auto& add_gate(gate_base op, std::string const& qlabel_target)
	{
		auto qid_target = qlabels_->to_qid(qlabel_target);
		gate_type gate(op, qid_target);
		return add_gate(gate);
	}

	/*! \brief Add a gate to the network. */
	auto& add_gate(gate_base op, std::string const& qlabel_control,
	               std::string const& qlabel_target)
	{
		auto qid_control = qlabels_->to_qid(qlabel_control);
		auto qid_target = qlabels_->to_qid(qlabel_target);
		gate_type gate(op, qid_control, qid_target);
		return add_gate(gate);
	}

	/*! \brief Add a gate to the network. */
	auto& add_gate(gate_base op, std::vector<std::string> const& qlabels_control,
	               std::vector<std::string> const& qlabels_target)
	{
		std::vector<uint32_t> qids_control;
		for (auto& control : qlabels_control) {
			qids_control.push_back(qlabels_->to_qid(control));
		}
		std::vector<uint32_t> qids_target;
		for (auto& target : qlabels_target) {
			qids_target.push_back(qlabels_->to_qid(target));
		}
		gate_type gate(op, qids_control, qids_target);
		return add_gate(gate);
	}
#pragma endregion

#pragma region Const iterators
	/*! \brief Calls ``fn`` on every qubit in the network.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following three signatures.
	 * - ``void(uint32_t qid)``
	 * - ``void(string const& qlabel)``
	 * - ``void(uint32_t qid, string const& qlabel)``
	 */
	template<typename Fn>
	void foreach_cqubit(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, uint32_t> ||
		              std::is_invocable_r_v<void, Fn, std::string const&> || 
			      std::is_invocable_r_v<void, Fn, uint32_t, std::string const&>);
		// clang-format on
		if constexpr (std::is_invocable_r_v<void, Fn, uint32_t>) {
			for (auto qid = 0u; qid < num_qubits(); ++qid) {
				fn(qid);
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, std::string const&>) {
			for (auto const& qlabel : *qlabels_) {
				fn(qlabel);
			}
		} else {
			auto qid = 0u;
			for (auto const& qlabel : *qlabels_) {
				fn(qid++, qlabel);
			}
		}
	}

	/*! \brief Calls ``fn`` on every input node in the network.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following two signatures.
	 * - ``void(node_type const& node)``
	 * - ``void(node_type const& node, uint32_t node_index)``
	 */
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

	/*! \brief Calls ``fn`` on every output node in the network.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following two signatures.
	 * - ``void(node_type const& node)``
	 * - ``void(node_type const& node, uint32_t node_index)``
	 */
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

	/*! \brief Calls ``fn`` on every unitrary gate node in the network.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following four signatures.
	 * - ``void(node_type const& node)``
	 * - ``void(node_type const& node, uint32_t node_index)``
	 * - ``bool(node_type const& node)``
	 * - ``bool(node_type const& node, uint32_t node_index)``
	 *
	 * If ``fn`` returns a ``bool``, then it can interrupt the iteration by returning ``false``.
	 */
	template<typename Fn>
	void foreach_cgate(Fn&& fn) const
	{
		foreach_element_if(storage_->nodes.cbegin(), storage_->nodes.cend(),
		                   [](auto const& node) { return node.gate.is_unitary_gate(); },
		                   fn);
	}

	/*! \brief Calls ``fn`` on every node in the network.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following four signatures.
	 * - ``void(node_type const& node)``
	 * - ``void(node_type const& node, uint32_t node_index)``
	 * - ``bool(node_type const& node)``
	 * - ``bool(node_type const& node, uint32_t node_index)``
	 *
	 * If ``fn`` returns a ``bool``, then it can interrupt the iteration by returning ``false``.
	 */
	template<typename Fn>
	void foreach_cnode(Fn&& fn) const
	{
		foreach_element(storage_->nodes.cbegin(), storage_->nodes.cend(), fn);
		foreach_element(storage_->outputs.cbegin(), storage_->outputs.cend(), fn,
		                storage_->nodes.size());
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

private:
	std::shared_ptr<storage_type> storage_;
	std::shared_ptr<qlabels_map> qlabels_;
};

} // namespace tweedledum
