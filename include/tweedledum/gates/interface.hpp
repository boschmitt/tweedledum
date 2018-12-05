/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

static_assert(false, "file interface.hpp cannot be included, it's only used for documentation");

namespace tweedledum {

class gate {
public:
#pragma region Constants
	constexpr static uint32_t max_num_qubits;
	constexpr static uint32_t network_max_num_qubits;
#pragma endregion

#pragma region Constructors
	/*! \brief Construct a single qubit gate
	 *
	 * \param op the operation (must be a single qubit operation).
	 * \param qid_target qubit identifier of the target.
	 */
	gate(gate_base const& op, uint32_t qid_target);

	/*! \brief Construct a controlled gate
	 *
	 * \param controlled_op the operation (must be a two qubit controlled operation).
	 * \param qid_control qubit identifier of the control.
	 * \param qid_target qubit identifier of the target.
	 */
	gate(gate_base const& controlled_op, uint32_t qid_control, uint32_t qid_target);

	/*! \brief Construct a gate using vectors
	 *
	 * \param unitary_op the operation (must be unitary operation).
	 * \param qids_control qubit(s) identifier of the control(s).
	 * \param qid_target qubit identifier of the target.
	 */
	gate(gate_base const& unitary_op, std::vector<uint32_t> const& qids_control,
	     std::vector<uint32_t> const& qid_target);
#pragma endregion

#pragma region Properties
	/*! \brief Return the number of controls */
	uint32_t num_controls() const;

	/*! \brief Returns the number of targets. */
	uint32_t num_targets() const;
#pragma endregion

#pragma region Iterators
	/*! \brief Calls ``fn`` on every target qubit of the gate.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following two signatures.
	 * - ``void(uint32_t)``
	 * - ``bool(uint32_t)``
	 *
	 * ``fn`` has one parameter: a qid. If ``fn`` returns a ``bool``, then it can interrupt the
	 * iteration by returning ``false``.
	 */
	template<typename Fn>
	void foreach_control(Fn&& fn) const;

	/*! \brief Calls ``fn`` on every target qubit of the gate.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following signature.
	 * - ``void(uint32_t)``
	 *
	 * ``fn`` has one parameter: a qid.
	 */
	template<typename Fn>
	void foreach_target(Fn&& fn) const;
#pragma endregion
};

} // namespace tweedledum