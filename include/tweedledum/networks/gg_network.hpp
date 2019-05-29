/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_base.hpp"
#include "../utils/foreach.hpp"
#include "detail/storage.hpp"
#include "io_id.hpp"

#include <array>
#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace tweedledum {

/*! Gate Graph (GG) is a directed acyclic graph (DAG) representation of a quantum circuit.
 *
 * The vertices in the graph are either input, output or operation vertecies.  All vertices store a
 * gate object, which is defined as a class template parameter---allowing great flexibility in the
 * types supported as gates.
 *
 * The arcs encode a input/output relationship between the gates.  That is, a arc from vertex A to
 * verterx B means that the qubit _must_ pass from the output of A to the input of B.
 */
template<typename GateType>
class gg_network {
public:
#pragma region Types and constructors
	using gate_type = GateType;
	using vertex_type = vertex<gate_type, 1>;
	using link_type = typename vertex_type::link_type;
	using storage_type = storage<vertex_type>;

	gg_network()
	    : storage_(std::make_shared<storage_type>())
	    , labels_(std::make_shared<labels_map>())
	{}

	explicit gg_network(std::string_view name)
	    : storage_(std::make_shared<storage_type>(name))
	    , labels_(std::make_shared<labels_map>())
	{}
#pragma endregion

#pragma region I / O and ancillae qubits
private:
	io_id create_io(bool is_qubit)
	{
		io_id id(storage_->inputs.size(), is_qubit);
		uint32_t index = storage_->vertices.size();
		gate_type input(gate_base(gate_lib::input), id);
		gate_type output(gate_base(gate_lib::output), id);

		storage_->vertices.emplace_back(input);
		storage_->inputs.emplace_back(index);
		vertex_type& output_vertex = storage_->outputs.emplace_back(output);
		output_vertex.children[0] = link_type(index);
		storage_->rewiring_map.push_back(id);
		return id;
	}

public:
	io_id add_qubit(std::string const& label)
	{
		io_id qid = create_io(true);
		labels_->map(qid, label);
		storage_->num_qubits += 1;
		return qid;
	}

	io_id add_qubit()
	{
		std::string label = fmt::format("q{}", num_qubits());
		return add_qubit(label);
	}

	io_id add_cbit(std::string const& label)
	{
		io_id id = create_io(false);
		labels_->map(id, label);
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
#pragma endregion

#pragma region Structural properties
	uint32_t size() const
	{
		return (storage_->vertices.size() + storage_->outputs.size());
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
		return (storage_->vertices.size() - storage_->inputs.size());
	}
#pragma endregion

#pragma region Vertices
	vertex_type& vertex(link_type link) const
	{
		return storage_->vertices[link];
	}

	vertex_type& vertex(uint32_t index) const
	{
		return storage_->vertices[index];
	}

	uint32_t index(vertex_type const& vertex) const
	{
		if (vertex.gate.is(gate_lib::output)) {
			auto index = &vertex - storage_->outputs.data();
			return static_cast<uint32_t>(index + storage_->vertices.size());
		}
		return static_cast<uint32_t>(&vertex - storage_->vertices.data());
	}
#pragma endregion

#pragma region Add gates(id)
private:
	void connect_vertex(io_id id, uint32_t index)
	{
		vertex_type& vertex = storage_->vertices.at(index);
		vertex_type& output = storage_->outputs.at(id.index());
		auto slot = vertex.gate.qubit_slot(id);

		assert(output.children[0] != link_type::max);
		foreach_child(output, [&vertex, slot](link_type link) {
			vertex.children[slot] = link;
		});
		output.children[0] =  link_type(index);
		return;
	}

public:
	template<typename... Args>
	vertex_type& emplace_gate(Args&&... args)
	{
		uint32_t index = storage_->vertices.size();
		vertex_type& vertex = storage_->vertices.emplace_back(std::forward<Args>(args)...);
		storage_->gate_set |= (1 << static_cast<uint32_t>(vertex.gate.operation()));
		vertex.gate.foreach_control([&](io_id id) { connect_vertex(id, index); });
		vertex.gate.foreach_target([&](io_id id) { connect_vertex(id, index); });
		return vertex;
	}

	vertex_type& add_gate(gate_base op, io_id target)
	{
		return emplace_gate(gate_type(op, storage_->rewiring_map.at(target)));
	}

	vertex_type& add_gate(gate_base op, io_id control, io_id target)
	{
		const io_id control_ = control.is_complemented() ?
		                           !storage_->rewiring_map.at(control) :
		                           storage_->rewiring_map.at(control);
		return emplace_gate(gate_type(op, control_, storage_->rewiring_map.at(target)));
	}

	vertex_type& add_gate(gate_base op, std::vector<io_id> controls, std::vector<io_id> targets)
	{
		std::transform(controls.begin(), controls.end(), controls.begin(),
		               [&](io_id id) -> io_id {
				       const io_id real_id = storage_->rewiring_map.at(id);
			               return id.is_complemented() ? !real_id : real_id;
		               });
		std::transform(targets.begin(), targets.end(), targets.begin(),
		               [&](io_id id) -> io_id {
			               return storage_->rewiring_map.at(id);
		               });
		return emplace_gate(gate_type(op, controls, targets));
	}
#pragma endregion

#pragma region Add gates(labels)
	vertex_type& add_gate(gate_base op, std::string const& qlabel_target)
	{
		auto qid_target = labels_->to_id(qlabel_target);
		return add_gate(op, qid_target);
	}

	vertex_type& add_gate(gate_base op, std::string const& qlabel_control,
	                    std::string const& qlabel_target)
	{
		auto qid_control = labels_->to_id(qlabel_control);
		auto qid_target = labels_->to_id(qlabel_target);
		return add_gate(op, qid_control, qid_target);
	}

	vertex_type& add_gate(gate_base op, std::vector<std::string> const& qlabels_control,
	                    std::vector<std::string> const& qlabels_target)
	{
		std::vector<io_id> controls;
		for (auto& control : qlabels_control) {
			controls.push_back(labels_->to_id(control));
		}
		std::vector<io_id> targets;
		for (auto& target : qlabels_target) {
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
		static_assert(std::is_invocable_r_v<void, Fn, vertex_type const&, uint32_t> ||
		              std::is_invocable_r_v<void, Fn, vertex_type const&>);
		// clang-format on
		for (uint32_t index : storage_->inputs) {
			if constexpr (std::is_invocable_r_v<void, Fn, vertex_type const&, uint32_t>) {
				fn(storage_->vertices[index], index);
			} else {
				fn(storage_->vertices[index]);
			}
		}
	}

	template<typename Fn>
	void foreach_output(Fn&& fn) const
	{
		// clang-format off
		static_assert(std::is_invocable_r_v<void, Fn, vertex_type const&, uint32_t> ||
		              std::is_invocable_r_v<void, Fn, vertex_type const&>);
		// clang-format on
		uint32_t index = storage_->vertices.size();
		for (vertex_type const& vertex : storage_->outputs) {
			if constexpr (std::is_invocable_r_v<void, Fn, vertex_type const&, uint32_t>) {
				fn(vertex, index++);
			} else {
				fn(vertex);
			}
		}
	}

	template<typename Fn>
	void foreach_gate(Fn&& fn, uint32_t start = 0) const
	{
		foreach_element_if(storage_->vertices.cbegin() + start, storage_->vertices.cend(),
		                   [](auto const& vertex) { return vertex.gate.is_gate(); },
		                   fn);
	}

	template<typename Fn>
	void foreach_vertex(Fn&& fn) const
	{
		foreach_element(storage_->vertices.cbegin(), storage_->vertices.cend(), fn);
		foreach_element(storage_->outputs.cbegin(), storage_->outputs.cend(), fn,
		                storage_->vertices.size());
	}
#pragma endregion

#pragma region Const vertex iterators
	template<typename Fn>
	void foreach_child(vertex_type const& vertex, Fn&& fn) const
	{
		static_assert(is_callable_without_index_v< Fn, link_type, void>
		|| is_callable_with_index_v<Fn, link_type, void>);

		for (auto i = 0u; i < vertex.children.size(); ++i) {
			if (vertex.children[i] == link_type::max) {
				continue;
			}
			if constexpr (is_callable_without_index_v<Fn, link_type, void>) {
				fn(vertex.children[i]);
			} else if constexpr (is_callable_with_index_v<Fn, link_type, void>) {
				fn(vertex.children[i], i);
			}
		}
	}
#pragma endregion

#pragma region Rewiring
	void rewire(std::vector<io_id> const& rewiring_map)
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

#pragma region Visited flags
	void clear_visited() const
	{
		std::for_each(storage_->vertices.begin(), storage_->vertices.end(),
		              [](vertex_type& vertex) { vertex.data[0] = 0; });
		std::for_each(storage_->outputs.begin(), storage_->outputs.end(),
		              [](vertex_type& vertex) { vertex.data[0] = 0; });
	}

	auto visited(vertex_type const& vertex) const
	{
		return vertex.data[0];
	}

	void set_visited(vertex_type const& vertex, uint32_t value) const
	{
		vertex.data[0] = value;
	}
#pragma endregion

private:
	std::shared_ptr<storage_type> storage_;
	std::shared_ptr<labels_map> labels_;
};

} // namespace tweedledum
