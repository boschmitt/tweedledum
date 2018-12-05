/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

static_assert(false, "file interface.hpp cannot be included, it's only used for documentation");

namespace tweedledum {

template<typename G>
class network final {
public:
#pragma region Types and constructors
	/*! \brief Type referring to itself.
	 *
	 * The ``base_type`` is the network type itself. It is required, because views may extend
	 * networks, and this type provides a way to determine the underlying network type.
	 */
	using base_type = network;

	/*! \brief Type representing a gate.
	 *
	 * A ``Gate`` is an operation that can be applied to a collection of qubits. It could be a
	 * meta operation, such as, primary input and a primary output, or a unitary operation gate.
	 */
	using gate_type = G;

	/*! \brief Type representing a node.
	 *
	 * A ``node`` is a node in the network. Each node must contains a gate.
	 */
	struct node_type {};

	/*! \brief Type representing the storage.
	 *
	 * A ``storage`` is some container that can contain all data necessary to store the network.
	 * It can constructed outside of the network and passed as a reference to the constructor.
	 * It may be shared among several networks. A `std::shared_ptr<T>` is a convenient data
	 * structure to hold a storage.
	 */
	struct storage_type {};

	network();

	explicit network(storage s);
#pragma endregion

#pragma region I / O and ancillae qubits
	/*! \brief Creates a labeled qubit in the network and returns its ``qid``
	 */
	auto add_qubit(std::string const& qlabel);

	/*! \brief Creates a unlabeled qubit in the network and returns its ``qid``
	 * 
	 * Since all qubits in a network must be labeled, this function will create
	 * a generic label with the form: qN, where N is the ``qid``.
	 */
	auto add_qubit();

#pragma endregion

#pragma region Structural properties
	/*! \brief Returns the number of nodes. */
	uint32_t size() const;

	/*! \brief Returns the number of qubits. */
	uint32_t num_qubits() const;

	/*! \brief Returns the number of gates, i.e., nodes that hold unitary operations */
	uint32_t num_gates() const;
#pragma endregion

#pragma region Add gates(qids)
	/*! \brief Add a gate to the network. */
	// This assumes the gate have been properly rewired
	node_type& add_gate(gate_type const& gate);

	/*! \brief Add a gate to the network. */
	node_type& add_gate(operation op, uint32_t qid_target, angle rotation_angle = 0.0);

	/*! \brief Add a gate to the network. */
	node_type& add_gate(operation op, uint32_t qid_control, uint32_t qid_target,
	                    angle rotation_angle = 0.0);

	/*! \brief Add a gate to the network. */
	node_type& add_gate(operation op, std::vector<uint32_t> const& qids_control,
	                    std::vector<uint32_t> const& qids_target, angle rotation_angle = 0.0);
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

#pragma region Const iterators
	/*! \brief Calls ``fn`` on every qubit in the network.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following three signatures.
	 * - ``void(uint32_t qid)``
	 * - ``void(string const& qlabel)``
	 * - ``void(uint32_t qid, string const& qlabel)``
	 */
	template<typename Fn>
	void foreach_cqubit(Fn&& fn) const;

	/*! \brief Calls ``fn`` on every input node in the network.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following two signatures.
	 * - ``void(node_type const& node)``
	 * - ``void(node_type const& node, uint32_t node_index)``
	 */
	template<typename Fn>
	void foreach_cinput(Fn&& fn) const;

	/*! \brief Calls ``fn`` on every output node in the network.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following two signatures.
	 * - ``void(node_type const& node)``
	 * - ``void(node_type const& node, uint32_t node_index)``
	 */
	template<typename Fn>
	void foreach_coutput(Fn&& fn) cons;

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
	void foreach_cgate(Fn&& fn) const;

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
	void foreach_cnode(Fn&& fn) const;
#pragma endregion

};

} // namespace tweedledum