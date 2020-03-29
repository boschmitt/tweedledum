/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "storage.hpp"
#include "wire_id.hpp"

namespace tweedledum {

/*! \brief Class used to represent a quantum circuit as a directed acyclic graph.
 *
 */
template<typename Operation, typename Node = node_regular<Operation>>
class op_dag {
public:
#pragma region Types and constructors
	using base_type = op_dag;
	using op_type = Operation;
	using node_type = Node;
	using dstrg_type = storage<node_type>;
	using wstrg_type = wire::storage;

	op_dag()
	    : data_(std::make_shared<dstrg_type>("tweedledum_op_graph"))
	    , wires_(std::make_shared<wstrg_type>())
	{}

	explicit op_dag(std::string_view name)
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

	void reserve(uint32_t new_cap)
	{
		data_->nodes.reserve(new_cap);
	}

	uint32_t num_operations() const
	{
		return (data_->nodes.size() - data_->inputs.size());
	}

	bool check_gate_set(uint64_t allowed_gates) const
	{
		return (data_->gate_set & ~allowed_gates) == 0ull;
	}
#pragma endregion

#pragma region Nodes
	node_id id(node_type const& n) const
	{
		return node_id(static_cast<uint32_t>(&n - data_->nodes.data()));
	}

	node_type const& node(node_id id) const
	{
		return data_->nodes.at(id);
	}
#pragma endregion

#pragma region Node custom values
	void default_value(uint32_t value) const
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
	void connect_wire(wire_id w_id)
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

	wire_id create_qubit(std::string const& name, wire_modes mode = wire_modes::inout)
	{
		wire_id w_id = wires_->create_qubit(name, mode);
		connect_wire(w_id);
		return w_id;
	}

	// This function is needed otherwise I cannot call create_qubit("<qubit_name>")
	wire_id create_qubit(char const* cstr_name, wire_modes mode = wire_modes::inout)
	{
		std::string name(cstr_name);
		return create_qubit(name, mode);
	}

	wire_id create_qubit(wire_modes mode = wire_modes::inout)
	{
		std::string name = fmt::format("__dum_q{}", num_qubits());
		return create_qubit(name, mode);
	}

	wire_id create_cbit(std::string const& name, wire_modes mode = wire_modes::inout)
	{
		wire_id w_id = wires_->create_cbit(name, mode);
		connect_wire(w_id);
		return w_id;
	}

	wire_id create_cbit(char const* cstr_name,  wire_modes mode = wire_modes::inout)
	{
		std::string name(cstr_name);
		return create_cbit(name, mode);
	}

	wire_id create_cbit(wire_modes mode = wire_modes::inout)
	{
		std::string name = fmt::format("__dum_c{}", num_cbits());
		return create_cbit(name, mode);
	}

	wire_id wire(std::string const& name) const
	{
		return wires_->wire(name);
	}

	std::string wire_name(wire_id w_id) const
	{
		return wires_->wire_name(w_id);
	}

	/* \brief Add a new name to identify a wire.
	 *
	 * \param rename If true, this flag indicates that `new_name` must substitute the previous
	 *               name. (default: `true`) 
	 */
	void wire_name(wire_id w_id, std::string const& new_name, bool rename = true)
	{
		wires_->wire_name(w_id, new_name, rename);
	}

	wire_modes wire_mode(wire_id w_id) const
	{
		return wires_->wire_mode(w_id);
	}

	void wire_mode(wire_id w_id, wire_modes new_mode)
	{
		wires_->wire_mode(w_id, new_mode);
	}
#pragma endregion

#pragma region Creating operations (using wire ids)
private:
	void connect_node(wire_id wire, node_type& node)
	{
		assert(data_->outputs.at(wire) != node::invalid);
		uint32_t position = node.op.position(wire);
		node.children.at(position) = data_->outputs.at(wire);
		data_->outputs.at(wire) = id(node);
		return;
	}

public:
	template<typename Op>
	node_id emplace_op(Op&& op)
	{
		node_id id(data_->nodes.size());
		node_type& node = data_->nodes.emplace_back(std::forward<Op>(op),
		                                               data_->default_value);
		data_->gate_set |= (1 << static_cast<uint32_t>(op.id()));
		node.op.foreach_control([&](wire_id wire) { connect_node(wire, node); });
		node.op.foreach_target([&](wire_id wire) { connect_node(wire, node); });
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

	node_id create_op(gate const& g, std::vector<wire_id> controls, std::vector<wire_id> targets)
	{
		return emplace_op(op_type(g, controls, targets));
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

	node_id create_op(gate const& g, std::vector<std::string> const& c_labels,
	                  std::vector<std::string> const& t_labels)
	{
		std::vector<wire_id> controls;
		for (std::string const& control : c_labels) {
			controls.push_back(wire(control));
		}
		std::vector<wire_id> targets;
		for (std::string const& target : t_labels) {
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
		static_assert(std::is_invocable_r_v<void, Fn, node_id> ||
		              std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id>);
		// clang-format on
		for (uint32_t i = 0u, i_limit = data_->inputs.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_id>) {
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
		static_assert(std::is_invocable_r_v<void, Fn, node_id> ||
		              std::is_invocable_r_v<void, Fn, node_type const&> ||
		              std::is_invocable_r_v<void, Fn, node_type const&, node_id>);
		// clang-format on
		for (uint32_t i = 0u, i_limit = data_->outputs.size(); i < i_limit; ++i) {
			if constexpr (std::is_invocable_r_v<void, Fn, node_id>) {
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
#pragma endregion

#pragma region Node iterators
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

private:
	std::shared_ptr<dstrg_type> data_;
	std::shared_ptr<wstrg_type> wires_;
};

} // namespace tweedledum
