/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

#include <string>
#include <vector>

namespace tweedledum {

static_assert(false,
              "file networks/interface.hpp cannot be included, it's only used for documentation");

class network final {
#pragma region Types and constructors
	/*! \brief Type representing a node.
	 *
	 * A ``node`` is a node in the network. It could be a input, a output or a quantum operator.
	 */
	struct node {};

	/*! \brief Type representing a pointer to a node.
	 *
	 * A ``node_ptr`` can be seen as a pointer to a node, or an outgoing edge of a node towards
	 * its parents, or a parent towards its children. Depending on the kind of network, it may
	 * carry additional information.
	 */
	struct node_ptr {};

	/*! \brief Type representing the storage.
	 *
	 * A ``storage`` is some container that can contain all data necessary to store the network.
	 * It can constructed outside of the network and passed as a reference to the constructor.
	 * It may be shared among several networks. A `std::shared_ptr<T>` is a convenient data
	 * structure to hold a storage in a network.
	 */
	struct storage {};

	network();

	explicit network(storage s);
#pragma endregion

#pragma region I / O and ancillae qubits
	/*! \brief Creates a qubit in the network.
	 *
	 * Each created qubit creates two nodes (input and ouput) and both
	 * contributes to the size of the network. A
	 */
	auto add_qubit();

	/*! \brief Creates a labeled qubit in the network.
	 *
	 * Each created qubit creates two nodes (input and ouput) and both contributes to the size
	 * of the network.
	 *
	 * \param qubit_label Optional name for the qubit
	 *
	 * If a label is not provided, one will be generated.
	 */
	auto add_qubit(std::string const& qubit_label = {});
#pragma endregion

#pragma region Structural properties
	/*! \brief Returns the number of nodes (including qubits). */
	auto size() const;

	/*! \brief Returns the number of qubits. */
	auto num_qubits() const;

	/*! \brief Returns the number of gates. */
	auto num_gates() const;
#pragma endregion

#pragma region Nodes
	/*! \brief Get the node a node_ptr is pointing to. */
	auto& get_node(node_ptr node_ptr) const;

	/*! \brief Returns the index of a node.
	 *
	 * The index of a node must be a unique for each node and must be between 0 (inclusive) and
	 * the size of a network (exclusive, value returned by ``size()``).
	 */
	auto node_to_index(node const& node) const;

	auto get_children(node const& node, uint32_t qubit_id);

	auto get_predecessor_choices(node const& node);
#pragma endregion

#pragma region Add gates
	auto& add_gate(gate g);

	auto& add_gate(gate_kinds_t kind, uint32_t target, float rotation_angle = 0.0);

	auto& add_gate(gate_kinds_t kind, uint32_t control, uint32_t target, float rotation_angle = 0.0);

	auto& add_gate(gate_kinds_t kind, std::vector<uint32_t> const& controls,
	               std::vector<uint32_t> const& targets, float rotation_angle = 0.0);

	auto& add_gate(gate_kinds_t kind, std::string const& target, float rotation_angle = 0.0);

	auto& add_gate(gate_kinds_t kind, std::vector<std::string> const& controls,
	               std::vector<std::string> const& targets, float rotation_angle = 0.0);
#pragma endregion

#pragma region Const node iterators
	template<typename Fn>
	void foreach_qubit(Fn&& fn) const;

	template<typename Fn>
	void foreach_input(Fn&& fn) const;

	template<typename Fn>
	void foreach_output(Fn&& fn) const;

	/*! \brief Calls ``fn`` on every node in network.
	 *
	 * The order of nodes depends on the implementation and must not guarantee
	 * topological order.  The paramater ``fn`` is any callable that must have
	 * one of the following four signatures.
	 * - ``void(node const&)``
	 * - ``void(node const&, uint32_t)``
	 * - ``bool(node const&)``
	 * - ``bool(node const&, uint32_t)``
	 *
	 * If ``fn`` has two parameters, the second parameter is an index starting
	 * from 0 and incremented in every iteration.  If ``fn`` returns a ``bool``,
	 * then it can interrupt the iteration by returning ``false``.
	 */
	template<typename Fn>
	void foreach_node(Fn&& fn) const;

	template<typename Fn>
	void foreach_gate(Fn&& fn) const;

	template<typename Fn>
	void foreach_child(node const& n, Fn&& fn) const;

	template<typename Fn>
	void foreach_child(node const& n, uint32_t qubit_id, Fn&& fn) const;
#pragma endregion

#pragma region Visited flags
	/*! \brief Reset all mark values to 0. */
	void clear_marks();

	/*! \brief Returns the marked value of a node. */
	auto mark(node const& node) const;

	/*! \brief Sets the marked value of a node. */
	void mark(node const& node, uint8_t value);

	void default_mark(std::uint8_t value);
#pragma endregion
};

} // namespace tweedledum
