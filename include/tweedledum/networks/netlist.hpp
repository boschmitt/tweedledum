/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_base.hpp"
#include "../utils/foreach.hpp"
#include "detail/storage.hpp"
#include "io_id.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <string>
#include <type_traits>

namespace tweedledum {

/*! \brief Netlist representation of a quantum circuit. */
template<typename GateType>
class netlist {
public:
#pragma region Types and constructors
	using base_type = netlist;
	using gate_type = GateType;
	using node_type = wrapper_vertex<gate_type, 1>;
	using link_type = typename node_type::link_type;
	using storage_type = storage<node_type>;

	netlist()
	    : storage_(std::make_shared<storage_type>())
	    , labels_(std::make_shared<labels_map>())
	{}

	explicit netlist(std::string_view name)
	    : storage_(std::make_shared<storage_type>(name))
	    , labels_(std::make_shared<labels_map>())
	{}
#pragma endregion

#pragma region I / O and ancillae qubits
private:
	auto create_io(bool is_qubit)
	{
		io_id id(storage_->inputs.size(), is_qubit);
		uint32_t index = storage_->nodes.size();
		gate_type input(gate_base(gate_lib::input), id);
		gate_type output(gate_base(gate_lib::output), id);

		storage_->nodes.emplace_back(input);
		storage_->inputs.emplace_back(index);
		storage_->outputs.emplace_back(output);
		storage_->wiring_map.push_back(id);
		return id;
	}

public:
	io_id add_qubit(std::string const& label, bool is_ancilla = false)
	{
		io_id qid = create_io(true);
		labels_->map(qid, label);
		storage_->num_qubits += 1;
		storage_->io_marks.push_back(is_ancilla ? 1 : 0);
		return qid;
	}

	io_id add_qubit(char const* c_label, bool is_ancilla = false)
	{
		std::string label(c_label);
		return add_qubit(label, is_ancilla);
	}

	io_id add_qubit(bool is_ancilla = false)
	{
		std::string label = fmt::format("q{}", num_qubits());
		return add_qubit(label, is_ancilla);
	}

	void mark_as_ancilla(io_id id)
	{
		storage_->io_marks.at(id) |= 1;
	}

	void unmark_as_ancilla(io_id id)
	{
		storage_->io_marks.at(id) &= 0xFE;
	}

	void mark_as_input(io_id id)
	{
		storage_->io_marks.at(id) |= (1 << 1);
	}

	void mark_as_output(io_id id)
	{
		storage_->io_marks.at(id) |= (1 << 2);
	}

	bool is_ancilla(io_id id) const
	{
		return storage_->io_marks.at(id) & 1;
	}

	bool is_input(io_id id) const
	{
		return storage_->io_marks.at(id) & (1 << 1);
	}

	bool is_output(io_id id) const
	{
		return storage_->io_marks.at(id) & (1 << 2);
	}

	io_id add_cbit(std::string const& label)
	{
		io_id id = create_io(false);
		labels_->map(id, label);
		storage_->io_marks.push_back(0u);
		return id;
	}

	io_id add_cbit()
	{
		std::string label = fmt::format("c{}", num_cbits());
		return add_cbit(label);
	}

	std::string io_label(io_id id) const
	{
		return labels_->to_label(id);
	}

	io_id id(std::string const& label) const
	{
		return labels_->to_id(label);
	}

	void io_set_label(io_id id, std::string const& label)
	{
		labels_->remap(id, label);
	}
#pragma endregion

#pragma region Properties
	std::string_view name() const
	{
		return storage_->name;
	}

	uint32_t gate_set() const 
	{
		return storage_->gate_set;
	}

private:
	template<typename... OPs>
	uint32_t unpack_allowed_gates(gate_lib op) const
	{
		const uint32_t pos = static_cast<uint32_t>(op);
		return (1 << pos);
	}

	template<typename... OPs>
	uint32_t unpack_allowed_gates(gate_lib op0, OPs... opN) const
	{
		return unpack_allowed_gates(op0) | unpack_allowed_gates(opN...);
	}

public:
	template<typename... OPs>
	bool check_gate_set(OPs... opN) const
	{
		uint32_t allowed = unpack_allowed_gates(opN...);
		return (storage_->gate_set & ~allowed) == 0;
	}
#pragma endregion

#pragma region Structural properties
	uint32_t size() const
	{
		return (storage_->nodes.size() + storage_->outputs.size());
	}

	uint32_t capacity() const
	{
		return storage_->nodes.capacity();
	}

	void reserve(uint32_t new_cap) const
	{
		storage_->nodes.reserve(new_cap);
	}

	uint32_t num_io() const
	{
		return (storage_->inputs.size());
	}

	uint32_t num_qubits() const
	{
		return (storage_->num_qubits);
	}

	uint32_t num_cbits() const
	{
		return (storage_->inputs.size() - num_qubits());
	}

	uint32_t num_gates() const
	{
		return (storage_->nodes.size() - storage_->inputs.size());
	}
#pragma endregion

#pragma region Nodes
	node_type& get_node(link_type link) const
	{
		return storage_->nodes[link];
	}

	node_type& get_node(uint32_t index) const
	{
		return storage_->nodes[index];
	}

	uint32_t index(node_type const& node) const
	{
		if (node.gate.is(gate_lib::output)) {
			auto index = &node - storage_->outputs.data();
			return static_cast<uint32_t>(index + storage_->nodes.size());
		}
		return static_cast<uint32_t>(&node - storage_->nodes.data());
	}
#pragma endregion

#pragma region Add gates(ids)
	template<typename... Args>
	node_type& emplace_gate(Args&&... args)
	{
		node_type& node = storage_->nodes.emplace_back(std::forward<Args>(args)...);
		storage_->gate_set |= (1 << static_cast<uint32_t>(node.gate.operation()));
		node.data[0] = storage_->default_value;
		return node;
	}

	node_type& add_gate(gate_base op, io_id target)
	{
		return emplace_gate(gate_type(op, storage_->wiring_map.at(target)));
	}

	node_type& add_gate(gate_base op, io_id control, io_id target)
	{
		const io_id control_ = control.is_complemented() ?
		                           !storage_->wiring_map.at(control) :
		                           storage_->wiring_map.at(control);
		return emplace_gate(gate_type(op, control_, storage_->wiring_map.at(target)));
	}

	node_type& add_gate(gate_base op, std::vector<io_id> controls, std::vector<io_id> targets)
	{
		std::transform(controls.begin(), controls.end(), controls.begin(),
		               [&](io_id id) -> io_id {
				       const io_id real_id = storage_->wiring_map.at(id);
			               return id.is_complemented() ? !real_id : real_id;
		               });
		std::transform(targets.begin(), targets.end(), targets.begin(),
		               [&](io_id id) -> io_id {
			               return storage_->wiring_map.at(id);
		               });
		return emplace_gate(gate_type(op, controls, targets));
	}
#pragma endregion

#pragma region Add gates(labels)
	node_type& add_gate(gate_base op, std::string const& label_target)
	{
		auto qid_target = labels_->to_id(label_target);
		return add_gate(op, qid_target);
	}

	node_type& add_gate(gate_base op, std::string const& label_control,
	                    std::string const& label_target)
	{
		auto qid_control = labels_->to_id(label_control);
		auto qid_target = labels_->to_id(label_target);
		return add_gate(op, qid_control, qid_target);
	}

	node_type& add_gate(gate_base op, std::vector<std::string> const& labels_control,
	                    std::vector<std::string> const& labels_target)
	{
		std::vector<io_id> controls;
		for (auto& control : labels_control) {
			controls.push_back(labels_->to_id(control));
		}
		std::vector<io_id> targets;
		for (auto& target : labels_target) {
			targets.push_back(labels_->to_id(target));
		}
		return add_gate(op, controls, targets);
	}
#pragma endregion

#pragma region Const iterators
	template<typename Fn>
	io_id foreach_io(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, io_id> ||
			      std::is_invocable_r_v<bool, Fn, io_id> ||
		              std::is_invocable_r_v<void, Fn, std::string const&> || 
			      std::is_invocable_r_v<void, Fn, io_id, std::string const&>);
		// clang-format on
		if constexpr (std::is_invocable_r_v<bool, Fn, io_id>) {
			for (auto const& [_, id] : *labels_) {
				if (!fn(id)) {
					return id;
				}
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, io_id>) {
			for (auto const& [_, id] : *labels_) {
				fn(id);
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, std::string const&>) {
			for (auto const& [label, _] : *labels_) {
				fn(label);
			}
		} else {
			for (auto const& [label, id] : *labels_) {
				fn(id, label);
			}
		}
		return io_invalid;
	}

	template<typename Fn>
	io_id foreach_qubit(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, io_id> ||
			      std::is_invocable_r_v<bool, Fn, io_id> ||
		              std::is_invocable_r_v<void, Fn, std::string const&> || 
			      std::is_invocable_r_v<void, Fn, io_id, std::string const&>);
		// clang-format on
		if constexpr (std::is_invocable_r_v<bool, Fn, io_id>) {
			for (auto const& [_, id] : *labels_) {
				if (id.is_qubit() && !fn(id)) {
					return id;
				}
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, io_id>) {
			for (auto const& [_, id] : *labels_) {
				if (id.is_qubit()) {
					fn(id);
				}
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, std::string const&>) {
			for (auto const& [label, id] : *labels_) {
				if (id.is_qubit()) {
					fn(label);
				}
			}
		} else {
			for (auto const& [label, id] : *labels_) {
				if (id.is_qubit()) {
					fn(id, label);
				}
			}
		}
		return io_invalid;
	}

	template<typename Fn>
	io_id foreach_cbit(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, io_id> ||
			      std::is_invocable_r_v<bool, Fn, io_id> ||
		              std::is_invocable_r_v<void, Fn, std::string const&> || 
			      std::is_invocable_r_v<void, Fn, io_id, std::string const&>);
		// clang-format on
		if constexpr (std::is_invocable_r_v<bool, Fn, io_id>) {
			for (auto const& [_, id] : *labels_) {
				if (!id.is_qubit() && !fn(id)) {
					return id;
				}
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, io_id>) {
			for (auto const& [_, id] : *labels_) {
				if (!id.is_qubit()) {
					fn(id);
				}
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, std::string const&>) {
			for (auto const& [label, id] : *labels_) {
				if (!id.is_qubit()) {
					fn(label);
				}
			}
		} else {
			for (auto const& [label, id] : *labels_) {
				if (!id.is_qubit()) {
					fn(id, label);
				}
			}
		}
		return io_invalid;
	}

	template<typename Fn>
	void foreach_input(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&, uint32_t> ||
		              std::is_invocable_r_v<void, Fn, node_type const&>);
		// clang-format on
		for (uint32_t index : storage_->inputs) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_type const&, uint32_t>) {
				fn(storage_->nodes[index], index);
			} else {
				fn(storage_->nodes[index]);
			}
		}
	}

	template<typename Fn>
	void foreach_output(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&, uint32_t> ||
		              std::is_invocable_r_v<void, Fn, node_type const&>);
		// clang-format on
		uint32_t index = storage_->nodes.size();
		for (node_type const& node : storage_->outputs) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_type const&, uint32_t>) {
				fn(node, index++);
			} else {
				fn(node);
			}
		}
	}

	template<typename Fn>
	void foreach_gate(Fn&& fn, uint32_t start = 0) const
	{
		foreach_element_if(storage_->nodes.cbegin() + start, storage_->nodes.cend(),
		                   [](auto const& node) { return node.gate.is_gate(); },
		                   fn);
	}

	template<typename Fn>
	void foreach_rgate(Fn&& fn, uint32_t start = 0) const
	{
		foreach_element_if(storage_->nodes.crbegin() + start, storage_->nodes.crend(),
		                  [](auto const& node) { return node.gate.is_gate(); },
		                  fn);
	}

	template<typename Fn>
	void foreach_vertex(Fn&& fn) const
	{
		foreach_element(storage_->nodes.cbegin(), storage_->nodes.cend(), fn);
		foreach_element(storage_->outputs.cbegin(), storage_->outputs.cend(), fn,
		                storage_->nodes.size());
	}
#pragma endregion

#pragma region Rewiring
	void rewire(std::vector<io_id> const& new_wiring)
	{
		storage_->wiring_map = new_wiring;
	}

	void rewire(std::vector<std::pair<uint32_t, uint32_t>> const& transpositions)
	{
		for (auto&& [i, j] : transpositions) {
			std::swap(storage_->wiring_map[i], storage_->wiring_map[j]);
		}
	}

	std::vector<io_id> wiring_map() const
	{
		return storage_->wiring_map;
	}
#pragma endregion

#pragma region Custom node values
	void set_default_value(uint32_t value) const
	{
		storage_->default_value = value;
	}

	void clear_values() const
	{
		std::for_each(storage_->nodes.begin(), storage_->nodes.end(),
		              [](node_type& node) { node.data[0] = 0; });
		std::for_each(storage_->outputs.begin(), storage_->outputs.end(),
		              [](node_type& node) { node.data[0] = 0; });
	}

	uint32_t value(node_type const& node) const
	{
		return node.data[0];
	}

	void set_value(node_type const& node, uint32_t value)
	{
		node.data[0] = value;
	}

	uint32_t incr_value(node_type const& node) const
	{
		return ++node.data[0];
	}

	uint32_t decr_value(node_type const& node) const
	{
		assert(node.data[0] > 0);
		return --node.data[0];
	}
#pragma endregion

private:
	std::shared_ptr<storage_type> storage_;
	std::shared_ptr<labels_map> labels_;
};

} // namespace tweedledum
