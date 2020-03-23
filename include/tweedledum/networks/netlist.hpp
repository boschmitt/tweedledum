/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "storage.hpp"
#include "wire_id.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <string>

namespace tweedledum {

/*! \brief Class used to represent a quantum circuit as a list of operations.
 *
 */
template<typename Operation>
class netlist {
public:
#pragma region Types and constructors
	using base_type = netlist;
	using op_type = Operation;
	using node_type = node_wrapper<op_type>;
	using storage_type = storage<node_type>;

	netlist()
	    : storage_(std::make_shared<storage_type>("tweedledum_netlist"))
	    , labels_(std::make_shared<labels_map>())
	{}

	explicit netlist(std::string_view name)
	    : storage_(std::make_shared<storage_type>(name))
	    , labels_(std::make_shared<labels_map>())
	{}
#pragma endregion

#pragma region Properties
	std::string_view name() const
	{
		return storage_->name;
	}

	uint32_t size() const
	{
		return storage_->nodes.size();
	}

	uint32_t capacity() const
	{
		return storage_->nodes.capacity();
	}

	void reserve(uint32_t new_cap)
	{
		storage_->nodes.reserve(new_cap);
	}

	uint32_t num_wires() const
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

	uint32_t num_operations() const
	{
		return (storage_->nodes.size() - storage_->inputs.size());
	}

	bool check_gate_set(uint64_t allowed_gates) const
	{
		return (storage_->gate_set & ~allowed_gates) == 0ull;
	}
#pragma endregion

#pragma region Nodes
	node_id id(node_type const& n) const
	{
		return node_id(static_cast<uint32_t>(&n - storage_->nodes.data()));
	}

	node_type const& node(node_id id) const
	{
		return storage_->nodes.at(id);
	}
#pragma endregion

#pragma region Node custom values
	void default_value(uint32_t value) const
	{
		storage_->default_value = value;
	}

	void clear_values() const
	{
		std::for_each(storage_->nodes.begin(), storage_->nodes.end(),
		              [](node_type& node) { node.data = 0u; });
	}

	uint32_t value(node_type const& node) const
	{
		return node.data;
	}

	void value(node_type const& node, uint32_t value) const
	{
		node.data = value;
	}

	uint32_t incr_value(node_type const& node) const
	{
		return ++node.data;
	}

	uint32_t decr_value(node_type const& node) const
	{
		assert(node.data[0] > 0);
		return --node.data;
	}
#pragma endregion

#pragma region Wires
private:
	wire_id create_wire(bool is_qubit)
	{
		wire_id w_id(storage_->inputs.size(), is_qubit);
		node_id n_id(storage_->nodes.size());
		op_type input(gate_lib::input, w_id);

		storage_->nodes.emplace_back(input, storage_->default_value);
		storage_->inputs.emplace_back(n_id);
		storage_->outputs.emplace_back(n_id);
		return w_id;
	}

public:
	wire_id create_qubit(std::string const& label, wire_modes mode = wire_modes::inout)
	{
		wire_id id = create_wire(/* is_qubit */ true);
		labels_->map(id, label);
		storage_->num_qubits += 1;
		storage_->wire_mode.push_back(mode);
		return id;
	}

	// This function is needed otherwise I cannot call create_qubit("<qubit_name>")
	wire_id create_qubit(char const* c_str_label, wire_modes mode = wire_modes::inout)
	{
		std::string label(c_str_label);
		return create_qubit(label, mode);
	}

	wire_id create_qubit(wire_modes mode = wire_modes::inout)
	{
		std::string label = fmt::format("__q{}", num_qubits());
		return create_qubit(label, mode);
	}

	wire_id create_cbit(std::string const& label)
	{
		wire_id id = create_wire(false);
		labels_->map(id, label);
		storage_->wire_mode.push_back(wire_modes::inout);
		return id;
	}

	wire_id create_cbit(char const* c_str_label)
	{
		std::string label(c_str_label);
		return create_cbit(label);
	}

	wire_id create_cbit()
	{
		std::string label = fmt::format("__c{}", num_cbits());
		return create_cbit(label);
	}

	wire_id wire(std::string const& label) const
	{
		return labels_->to_id(label);
	}

	std::string wire_label(wire_id id) const
	{
		return labels_->to_label(id.wire());
	}

	void wire_label(wire_id id, std::string const& new_label) const
	{
		return labels_->remap(id.wire(), new_label);
	}

	wire_modes wire_mode(wire_id id) const
	{
		return storage_->wire_mode.at(id);
	}

	void wire_mode(wire_id id, wire_modes new_mode)
	{
		storage_->wire_mode.at(id) = new_mode;
	}
#pragma endregion

#pragma region Creating operations (using wire ids)
	template<typename Op>
	node_id emplace_op(Op&& op)
	{
		node_id id(storage_->nodes.size());
		storage_->gate_set |= (1 << static_cast<uint32_t>(op.gate.id()));
		storage_->nodes.emplace_back(std::forward<Op>(op), storage_->default_value);
		return id;
	}

	node_id create_op(gate const& g, wire_id t)
	{
		return emplace_op(op_type(g, t));
	}

	node_id create_op(gate const& g, wire_id w0, wire_id w1)
	{
		return emplace_op(op_type(g, w0, w1));
	}

	node_id create_op(gate const& g, wire_id c0, wire_id c1, wire_id t)
	{
		return emplace_op(op_type(g, c0, c1, t));
	}

	node_id create_op(gate const& g, std::vector<wire_id> cs, std::vector<wire_id> ts)
	{
		return emplace_op(op_type(g, cs, ts));
	}
#pragma endregion

#pragma region Creating operations (using wire labels)
	node_id create_op(gate const& g, std::string const& target)
	{
		return create_op(g, wire(target));
	}

	node_id create_op(gate const& g, std::string const& l0, std::string const& l1)
	{
		return create_op(g, wire(l0), wire(l1));
	}

	node_id create_op(gate const& g, std::string const& c0, std::string const& c1,
	                  std::string const& t)
	{
		return create_op(g, wire(c0), wire(c1), wire(t));
	}

	node_id create_op(gate const& g, std::vector<std::string> const& cs,
	                  std::vector<std::string> const& ts)
	{
		std::vector<wire_id> controls;
		for (std::string const& control : cs) {
			controls.push_back(wire(control));
		}
		std::vector<wire_id> targets;
		for (std::string const& target : ts) {
			targets.push_back(wire(target));
		}
		return create_op(g, controls, targets);
	}
#pragma endregion

#pragma region Iterators
	template<typename Fn>
	wire_id foreach_wire(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, wire_id> ||
			      std::is_invocable_r_v<bool, Fn, wire_id> ||
		              std::is_invocable_r_v<void, Fn, std::string const&> || 
			      std::is_invocable_r_v<void, Fn, wire_id, std::string const&>);
		// clang-format on
		if constexpr (std::is_invocable_r_v<bool, Fn, wire_id>) {
			for (auto const& [_, id] : *labels_) {
				if (!fn(id)) {
					return id;
				}
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, wire_id>) {
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
		return wire::invalid;
	}

	template<typename Fn>
	void foreach_input(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_id> ||
		              std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id>);
		// clang-format on
		for (uint32_t i = 0u, i_limit = storage_->inputs.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_id>) {
				fn(storage_->inputs.at(i));
			} else if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
				fn(node(storage_->inputs.at(i)));
			} else {
				fn(node(storage_->inputs.at(i)), storage_->inputs.at(i));
			}
		}
	}

	template<typename Fn>
	void foreach_output(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_id> ||
		              std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id>);
		// clang-format on
		for (uint32_t i = 0u, i_limit = storage_->outputs.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_id>) {
				fn(storage_->outputs.at(i));
			} else if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
				fn(node(storage_->outputs.at(i)));
			} else {
				fn(node(storage_->outputs.at(i)), storage_->outputs.at(i));
			}
		}
	}

	template<typename Fn>
	void foreach_op(Fn&& fn) const
	{
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&>);
		for (uint32_t i = 0u, i_limit = storage_->nodes.size(); i < i_limit; ++i) {
			node_type const& n = storage_->nodes.at(i);
			if (n.operation.gate.is_meta()) {
				continue;
			}
			fn(n);
		}
	}

	template<typename Fn>
	void foreach_rop(Fn&& fn) const
	{
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&>);
		for (uint32_t i = storage_->nodes.size(); i --> 0u;) {
			node_type const& n = storage_->nodes.at(i);
			if (n.operation.gate.is_meta()) {
				continue;
			}
			fn(n);
		}
	}
#pragma endregion

#pragma region Operation iterators
	template<typename Fn>
	void foreach_child(node_type const& n, Fn&& fn) const
	{
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id>);
		node_id n_id = id(n);
		if (n_id == 0u) {
			return;
		}
		node_id prev_id(static_cast<uint32_t>(n_id) - 1);
		if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
			fn(node(prev_id));
		} else {
			fn(node(prev_id), prev_id);
		}
	}
#pragma endregion

private:
	std::shared_ptr<storage_type> storage_;
	std::shared_ptr<labels_map> labels_;
};

} // namespace tweedledum
