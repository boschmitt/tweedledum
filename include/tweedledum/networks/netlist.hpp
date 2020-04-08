/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "storage.hpp"
#include "wire.hpp"

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
	using dstrg_type = storage<node_type>;
	using wstrg_type = wire::storage;

	netlist()
	    : data_(std::make_shared<dstrg_type>("tweedledum_netlist"))
	    , wires_(std::make_shared<wstrg_type>())
	{}

	explicit netlist(std::string_view name)
	    : data_(std::make_shared<dstrg_type>(name))
	    , wires_(std::make_shared<wstrg_type>())
	{}
#pragma endregion

#pragma region Properties
	std::string_view name() const
	{
		return data_->name;
	}

	uint32_t size() const
	{
		return data_->nodes.size();
	}

	uint32_t capacity() const
	{
		return data_->nodes.capacity();
	}

	void reserve(uint32_t const new_cap)
	{
		data_->nodes.reserve(new_cap);
	}

	uint32_t num_operations() const
	{
		return (data_->nodes.size() - data_->inputs.size());
	}

	bool check_gate_set(uint64_t const allowed_gates) const
	{
		return (data_->gate_set & ~allowed_gates) == 0ull;
	}
#pragma endregion

#pragma region Nodes
	node_id id(node_type const& n) const
	{
		return node_id(static_cast<uint32_t>(&n - data_->nodes.data()));
	}

	node_type const& node(node_id const id) const
	{
		return data_->nodes.at(id);
	}
#pragma endregion

#pragma region Node custom values
	void default_value(uint32_t const value) const
	{
		data_->default_value = value;
	}

	void clear_values() const
	{
		std::for_each(data_->nodes.begin(), data_->nodes.end(),
		              [](node_type& node) { node.data = 0u; });
	}

	uint32_t value(node_type const& node) const
	{
		return node.data;
	}

	void value(node_type const& node, uint32_t const value) const
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
	void connect_wire(wire::id const w_id)
	{
		node_id n_id(data_->nodes.size());
		op_type input(gate_lib::input, w_id);
		data_->nodes.emplace_back(input, data_->default_value);
		data_->inputs.emplace_back(n_id);
		data_->outputs.emplace_back(n_id);
	}

public:
	uint32_t num_wires() const
	{
		return wires_->num_wires();
	}

	uint32_t num_qubits() const
	{
		return wires_->num_qubits();
	}

	uint32_t num_cbits() const
	{
		return wires_->num_cbits();
	}

	wire::id create_qubit(std::string_view name, wire::modes const mode = wire::modes::inout)
	{
		wire::id const w_id = wires_->create_qubit(name, mode);
		connect_wire(w_id);
		return w_id;
	}

	wire::id create_qubit(wire::modes const mode = wire::modes::inout)
	{
		std::string const name = fmt::format("__dum_q{}", num_qubits());
		return create_qubit(name, mode);
	}

	wire::id create_cbit(std::string_view name, wire::modes const mode = wire::modes::inout)
	{
		wire::id const w_id = wires_->create_cbit(name, mode);
		connect_wire(w_id);
		return w_id;
	}

	wire::id create_cbit(wire::modes const mode = wire::modes::inout)
	{
		std::string const name = fmt::format("__dum_c{}", num_cbits());
		return create_cbit(name, mode);
	}

	wire::id wire(std::string_view name) const
	{
		return wires_->wire(name);
	}

	std::string wire_name(wire::id const w_id) const
	{
		return wires_->wire_name(w_id);
	}

	void wire_name(wire::id const w_id, std::string_view new_name, bool const rename = true)
	{
		wires_->wire_name(w_id, new_name, rename);
	}

	wire::modes wire_mode(wire::id const w_id) const
	{
		return wires_->wire_mode(w_id);
	}

	void wire_mode(wire::id const w_id, wire::modes const new_mode)
	{
		wires_->wire_mode(w_id, new_mode);
	}
#pragma endregion

#pragma region Creating operations (using wire ids)
	template<typename Op>
	node_id emplace_op(Op&& op)
	{
		node_id const id(data_->nodes.size());
		data_->gate_set |= (1 << static_cast<uint32_t>(op.id()));
		data_->nodes.emplace_back(std::forward<Op>(op), data_->default_value);
		return id;
	}

	node_id create_op(gate const& g, wire::id const w)
	{
		return emplace_op(op_type(g, w));
	}

	node_id create_op(gate const& g, wire::id const w0, wire::id const w1)
	{
		return emplace_op(op_type(g, w0, w1));
	}

	node_id create_op(gate const& g, wire::id const w0, wire::id const w1, wire::id const w2)
	{
		return emplace_op(op_type(g, w0, w1, w2));
	}

	node_id create_op(gate const& g, std::vector<wire::id> const& cs,
	                  std::vector<wire::id> const& ts)
	{
		return emplace_op(op_type(g, cs, ts));
	}
#pragma endregion

#pragma region Creating operations (using wire names)
	node_id create_op(gate const& g, std::string_view n)
	{
		return create_op(g, wire(n));
	}

	node_id create_op(gate const& g, std::string_view n0, std::string_view n1)
	{
		return create_op(g, wire(n0), wire(n1));
	}

	node_id create_op(gate const& g, std::string_view n0, std::string_view n1, std::string_view n2)
	{
		return create_op(g, wire(n0), wire(n1), wire(n2));
	}

	node_id create_op(gate const& g, std::vector<std::string> const& cs,
	                  std::vector<std::string> const& ts)
	{
		std::vector<wire::id> controls;
		for (std::string_view control : cs) {
			controls.push_back(wire(control));
		}
		std::vector<wire::id> targets;
		for (std::string_view target : ts) {
			targets.push_back(wire(target));
		}
		return create_op(g, controls, targets);
	}
#pragma endregion

#pragma region Iterators
	template<typename Fn>
	void foreach_wire(Fn&& fn) const
	{
		wires_->foreach_wire(std::forward<Fn>(fn));
	}

	template<typename Fn>
	void foreach_input(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_id const> ||
		              std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id const>);
		// clang-format on
		for (uint32_t i = 0u, i_limit = data_->inputs.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_id const>) {
				fn(data_->inputs.at(i));
			} else if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
				fn(node(data_->inputs.at(i)));
			} else {
				fn(node(data_->inputs.at(i)), data_->inputs.at(i));
			}
		}
	}

	template<typename Fn>
	void foreach_output(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_id const> ||
		              std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id const>);
		// clang-format on
		for (uint32_t i = 0u, i_limit = data_->outputs.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_id const>) {
				fn(data_->outputs.at(i));
			} else if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
				fn(node(data_->outputs.at(i)));
			} else {
				fn(node(data_->outputs.at(i)), data_->outputs.at(i));
			}
		}
	}

	template<typename Fn>
	void foreach_op(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, op_type const&> ||
		              std::is_invocable_r_v<void, Fn, op_type const&, node_type const&>);
		// clang-format on
		for (uint32_t i = 0u, i_limit = data_->nodes.size(); i < i_limit; ++i) {
			node_type const& node = data_->nodes.at(i);
			if (node.op.is_meta()) {
				continue;
			}
			if constexpr (std::is_invocable_r_v<void, Fn, op_type const&>) {
				fn(node.op);
			} else {
				fn(node.op, node);
			}
		}
	}

	template<typename Fn>
	void foreach_rop(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, op_type const&> ||
		              std::is_invocable_r_v<void, Fn, op_type const&, node_type const&>);
		// clang-format on
		for (uint32_t i = data_->nodes.size(); i --> 0u;) {
			node_type const& node = data_->nodes.at(i);
			if (node.op.is_meta()) {
				continue;
			}
			if constexpr (std::is_invocable_r_v<void, Fn, op_type const&>) {
				fn(node.op);
			} else {
				fn(node.op, node);
			}
		}
	}
	template<typename Fn>
	void foreach_node(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id const>);
		// clang-format on
		for (uint32_t i = 0u, i_limit = data_->nodes.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
				fn(data_->nodes.at(i));
			} else {
				fn(data_->nodes.at(i), node_id(i));
			}
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
	std::shared_ptr<dstrg_type> data_;
	std::shared_ptr<wstrg_type> wires_;
};

} // namespace tweedledum
