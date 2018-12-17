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
		qubit_id qid(storage_->inputs.size());
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

#pragma region Add gates(qids)
	auto& add_gate(gate_type const& gate)
	{
		// assert(!gate.is_meta());
		auto& node = storage_->nodes.emplace_back(gate);
		return node;
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
		                   [](auto const& node) { return node.gate.is_unitary_gate(); },
		                   fn);
	}

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
