/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "../operations/w2_op.hpp"
#include "../utils/device.hpp"
#include "storage.hpp"
#include "wire_id.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fmt/format.h>
#include <limits>
#include <random>
#include <string>

namespace tweedledum {

/*! \brief 
 */
class mapped_dag {
#pragma region Types and constructors
	struct map_storage_type {
		// Maps for orignal wire -> new wire
		std::vector<wire_id> wire_to_v;
		std::vector<wire_id> init_v_to_phy;
		std::vector<wire_id> v_to_phy;
		bit_matrix_rm<> coupling_matrix;

		map_storage_type(uint32_t num_wires, device const& arch)
		    : wire_to_v(num_wires, wire::invalid)
		    , coupling_matrix(arch.get_coupling_matrix())
		{}
	};

public:
	using base_type = mapped_dag;
	using op_type = w2_op;
	using node_type = node_regular<w2_op>;
	using node_storage_type = storage<node_type>;
	using wire_storage_type = labels_map;

	template<typename Network>
	mapped_dag(Network const& network, device const& arch)
	    : node_storage_(std::make_shared<node_storage_type>("tweedledum_mapd_network"))
	    , wire_storage_(std::make_shared<wire_storage_type>())
	    , map_storage_(std::make_shared<map_storage_type>(network.num_wires(), arch))
	{
		network.foreach_wire([&](wire_id wire, std::string const& label) {
			if (wire.is_qubit()) {
				map_storage_->wire_to_v.at(wire) = create_qubit(label);
			}
		});
		for (uint32_t i = network.num_qubits(); i < arch.num_vertices(); ++i) {
			create_qubit();
		}
		network.foreach_wire([&](wire_id wire, std::string const& label) {
			if (!wire.is_qubit()) {
				map_storage_->wire_to_v.at(wire) = create_cbit(label);
			}
		});
	}
#pragma endregion

#pragma region Properties
	std::string_view name() const
	{
		return node_storage_->name;
	}

	uint32_t size() const
	{
		return node_storage_->nodes.size();
	}

	uint32_t capacity() const
	{
		return node_storage_->nodes.capacity();
	}

	void reserve(uint32_t new_cap)
	{
		node_storage_->nodes.reserve(new_cap);
	}

	uint32_t num_wires() const
	{
		return (node_storage_->inputs.size());
	}

	uint32_t num_qubits() const
	{
		return (node_storage_->num_qubits);
	}

	uint32_t num_cbits() const
	{
		return (node_storage_->inputs.size() - num_qubits());
	}

	uint32_t num_operations() const
	{
		return (node_storage_->nodes.size() - node_storage_->inputs.size());
	}

	bool check_gate_set(uint64_t allowed_gates) const
	{
		return (node_storage_->gate_set & ~allowed_gates) == 0ull;
	}
#pragma endregion

#pragma region Nodes
	node_id id(node_type const& n) const
	{
		return node_id(static_cast<uint32_t>(&n - node_storage_->nodes.data()));
	}

	node_type const& node(node_id id) const
	{
		return node_storage_->nodes.at(id);
	}
#pragma endregion

#pragma region Node custom values
	void default_value(uint32_t value) const
	{
		node_storage_->default_value = value;
	}

	void clear_values() const
	{
		std::for_each(node_storage_->nodes.begin(), node_storage_->nodes.end(),
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
		assert(node.data > 0);
		return --node.data;
	}
#pragma endregion

#pragma region Wires
private:
	wire_id create_wire(bool is_qubit)
	{
		wire_id w_id(node_storage_->inputs.size(), is_qubit);
		node_id n_id(node_storage_->nodes.size());
		op_type input(gate_lib::input, w_id);

		node_storage_->nodes.emplace_back(input, node_storage_->default_value);
		node_storage_->inputs.emplace_back(n_id);
		node_storage_->outputs.emplace_back(n_id);
		map_storage_->init_v_to_phy.push_back(w_id);
		map_storage_->v_to_phy.push_back(w_id);
		return w_id;
	}

	wire_id create_qubit(std::string const& label, wire_modes mode = wire_modes::inout)
	{
		wire_id id = create_wire(/* is_qubit */ true);
		wire_storage_->map(id, label);
		node_storage_->num_qubits += 1;
		node_storage_->wire_mode.push_back(mode);
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

public:
	wire_id create_cbit(std::string const& label)
	{
		wire_id id = create_wire(false);
		wire_storage_->map(id, label);
		node_storage_->wire_mode.push_back(wire_modes::inout);
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
		return wire_storage_->to_id(label);
	}

	std::string wire_label(wire_id id) const
	{
		return wire_storage_->to_label(id.wire());
	}

	void wire_label(wire_id id, std::string const& new_label) const
	{
		return wire_storage_->remap(id.wire(), new_label);
	}

	wire_modes wire_mode(wire_id id) const
	{
		return node_storage_->wire_mode.at(id);
	}

	void wire_mode(wire_id id, wire_modes new_mode)
	{
		node_storage_->wire_mode.at(id) = new_mode;
	}
#pragma endregion

#pragma region Creating operations (using wire ids)
private:
	void connect_node(wire_id wire, node_type& n)
	{
		assert(node_storage_->outputs.at(wire) != node::invalid);
		uint32_t position = n.operation.position(wire);
		n.children.at(position) = node_storage_->outputs.at(wire);
		node_storage_->outputs.at(wire) = id(n);
		return;
	}

public:
	template<typename Op>
	node_id emplace_op(Op&& op)
	{
		node_id id(node_storage_->nodes.size());
		node_type& n = node_storage_->nodes.emplace_back(std::forward<Op>(op),
		                                                 node_storage_->default_value);
		node_storage_->gate_set |= (1 << static_cast<uint32_t>(op.gate.id()));
		n.operation.foreach_control([&](wire_id wire) { connect_node(wire, n); });
		n.operation.foreach_target([&](wire_id wire) { connect_node(wire, n); });
		return id;
	}

	node_id create_op(gate const& g, wire_id t)
	{
		return emplace_op(op_type(g, map_storage_->v_to_phy.at(t)));
	}

	node_id create_op(gate const& g, wire_id w0_v, wire_id w1_v)
	{
		wire_id w0_phy = map_storage_->v_to_phy.at(w0_v);
		wire_id w1_phy = map_storage_->v_to_phy.at(w1_v);
		if (!map_storage_->coupling_matrix.at(w0_phy, w1_phy)) {
			return node::invalid;
		}
		if (w0_v.is_complemented()) {
			w0_phy.complement();
		}
		return emplace_op(op_type(g, w0_phy, w1_phy));
	}

	node_id create_op(gate const& g, std::vector<wire_id> controls, std::vector<wire_id> targets)
	{
		wire_id w0_phy = wire::invalid;
		wire_id w1_phy = map_storage_->v_to_phy.at(targets.at(0));
		if (controls.size() + targets.size() > 2u) {
			return node::invalid;
		}
		if (controls.empty() && targets.size() == 1) {
			return emplace_op(op_type(g, w1_phy));
		}
		if (!controls.empty()) {
			w0_phy = map_storage_->v_to_phy.at(controls.at(0));
			if (controls.at(0).is_complemented()) {
				w0_phy.complement();
			}
		} else {
			assert(targets.size() == 2u);
			w0_phy = map_storage_->v_to_phy.at(targets.at(1));
		}
		if (!map_storage_->coupling_matrix.at(w0_phy, w1_phy)) {
			return node::invalid;
		}
		return emplace_op(op_type(g, w0_phy, w1_phy));
	}
#pragma endregion

#pragma region Add operations (using wire ids from the original network)
	node_id add_op(gate const& g, wire_id t)
	{
		assert(t < map_storage_->wire_to_v.size());
		wire_id t_v = map_storage_->wire_to_v.at(t);
		return create_op(g, t_v);
	}

	node_id add_op(gate const& g, wire_id w0, wire_id w1)
	{
		assert(w0 < map_storage_->wire_to_v.size());
		assert(w1 < map_storage_->wire_to_v.size());
		wire_id w0_v = map_storage_->wire_to_v.at(w0);
		wire_id w1_v = map_storage_->wire_to_v.at(w1);
		return create_op(g, w0_v, w1_v);
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
			for (auto const& [_, id] : *wire_storage_) {
				if (!fn(id)) {
					return id;
				}
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, wire_id>) {
			for (auto const& [_, id] : *wire_storage_) {
				fn(id);
			}
		} else if constexpr (std::is_invocable_r_v<void, Fn, std::string const&>) {
			for (auto const& [label, _] : *wire_storage_) {
				fn(label);
			}
		} else {
			for (auto const& [label, id] : *wire_storage_) {
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
		for (uint32_t i = 0u, i_limit = node_storage_->inputs.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_id>) {
				fn(node_storage_->inputs.at(i));
			} else if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
				fn(node(node_storage_->inputs.at(i)));
			} else {
				fn(node(node_storage_->inputs.at(i)), node_storage_->inputs.at(i));
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
		for (uint32_t i = 0u, i_limit = node_storage_->outputs.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_id>) {
				fn(node_storage_->outputs.at(i));
			} else if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
				fn(node(node_storage_->outputs.at(i)));
			} else {
				fn(node(node_storage_->outputs.at(i)), node_storage_->outputs.at(i));
			}
		}
	}

	template<typename Fn>
	void foreach_op(Fn&& fn) const
	{
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&>);
		for (uint32_t i = 0u, i_limit = node_storage_->nodes.size(); i < i_limit; ++i) {
			node_type const& n = node_storage_->nodes.at(i);
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
		for (uint32_t i = node_storage_->nodes.size(); i --> 0u;) {
			node_type const& n = node_storage_->nodes.at(i);
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
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, wire_id>);
		// clang-format on
		for (uint32_t position = 0u; position < n.children.size(); ++position) {
			if (n.children[position] == node::invalid) {
				continue;
			}
			if constexpr (std::is_invocable_r_v<void, Fn, node_type const&>) {
				fn(node(n.children[position]));
			} else if constexpr (std::is_invocable_r_v<void, Fn, node_type const&, node_id>) {
				fn(node(n.children[position]), n.children[position]);
			} else if constexpr (std::is_invocable_r_v<void, Fn, node_type const&, wire_id>) {
				fn(node(n.children[position]), n.operation.wire(position));
			}
		}
	}
#pragma endregion

#pragma region Mapping
	void create_swap(wire_id w0_phy, wire_id w1_phy)
	{
		assert(w0_phy.id() != w1_phy.id());
		assert(!w0_phy.is_complemented() && !w1_phy.is_complemented());
		assert(map_storage_->coupling_matrix.at(w0_phy, w1_phy));
		this->emplace_op(op_type(gate_lib::swap, w0_phy, w1_phy));
		uint32_t w0_idx;
		uint32_t w1_idx;
		uint32_t end_idx = map_storage_->v_to_phy.size();
		for (uint32_t i = 0, found = 0; i < end_idx && found != 2u; ++i) {
			if (map_storage_->v_to_phy.at(i) == w0_phy) {
				w0_idx = i;
				++found;
			} else if (map_storage_->v_to_phy.at(i) == w1_phy) {
				w1_idx = i;
				++found;
			}
		}
		std::swap(map_storage_->v_to_phy.at(w0_idx), map_storage_->v_to_phy.at(w1_idx));
	}

	wire_id wire_to_v(wire_id wire) const 
	{
		assert(wire.is_qubit());
		return map_storage_->wire_to_v.at(wire);
	}

	wire_id wire_to_phy(wire_id wire) const
	{
		assert(wire.is_qubit());
		return map_storage_->v_to_phy.at(map_storage_->wire_to_v.at(wire));
	}

	void v_to_phy(std::vector<wire_id> const& mapping)
	{
		assert(mapping.size() == num_qubits());
		if (num_operations() == 0) {
			std::copy(mapping.begin(), mapping.end(), map_storage_->init_v_to_phy.begin());
		}
		std::copy(mapping.begin(), mapping.end(), map_storage_->v_to_phy.begin());
	}

	wire_id v_to_phy(wire_id id) const
	{
		return map_storage_->v_to_phy.at(id);
	}

	std::vector<wire_id> v_to_phy() const
	{
		return {map_storage_->v_to_phy.begin(), map_storage_->v_to_phy.begin() + num_qubits()};
	}

	std::vector<wire_id> phy_to_v() const 
	{
		std::vector<wire_id> map(map_storage_->v_to_phy.size(), wire::invalid);
		for (uint32_t i = 0; i < map_storage_->v_to_phy.size(); ++i) {
			map[map_storage_->v_to_phy[i]] = wire_id(i, true);
		}
		return map;
	}

	std::vector<wire_id> init_phy_to_v() const 
	{
		std::vector<wire_id> map(num_qubits(), wire::invalid);
		for (uint32_t i = 0; i < num_qubits(); ++i) {
			map[map_storage_->init_v_to_phy[i]] = wire_id(i, true);
		}
		return map;
	}
#pragma endregion

private:
	std::shared_ptr<node_storage_type> node_storage_;
	std::shared_ptr<wire_storage_type> wire_storage_;
	std::shared_ptr<map_storage_type> map_storage_;
};

} // namespace tweedledum
