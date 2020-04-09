/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "../networks/wire.hpp"

static_assert(false,
              "file operations/interface.hpp cannot be included, it's only used for documentation");

namespace tweedledum {

class operation {
public:
#pragma region Constants
	constexpr static uint32_t max_num_wires;
	constexpr static uint32_t network_max_num_qubits;
#pragma endregion

#pragma region Constructors
	/*! \brief Construct an one-wire operation
	 *
	 * \param g A gate from ``gate_lib``.
	 * \param t Wire identifier of the target.
	 */
	operation(gate const& g, wire::id t);

	/*! \brief Construct a two wire operation
	 *
	 * \param g A gate from ``gate_lib``.
	 * \param w0 Wire identifier.
	 * \param w1 Wire identifier.
	 */
	operation(gate const& g, wire::id w0, wire::id w1);

	/*! \brief Construct a two wire operation
	 *
	 * \param g A gate from ``gate_lib``.
	 * \param c0 Wire identifier of the firt control.
	 * \param c1 Wire identifier of the second control.
	 * \param t Wire identifier of the target.
	 */
	operation(gate const& g, wire::id c0, wire::id c1, wire::id t);

	/*! \brief Construct a operation using vectors
	 *
	 * \param g A gate from ``gate_lib``.
	 * \param controls Wire identifier(s) of the control(s).
	 * \param targets Wire identifier(s) of the target(s).
	 */
	operation(gate const& g, std::vector<wire::id> const& controls,
	          std::vector<wire::id> const& targets);
#pragma endregion

#pragma region Properties
	/*! \brief Returns the number of controls. */
	uint32_t num_wires() const;

	/*! \brief Returns the number of controls. */
	uint32_t num_controls() const;

	/*! \brief Returns the number of targets. */
	uint32_t num_targets() const;

	/*! \brief Returns the i-th control wire identifier.
	 *
	 * \param i must be less than ``num_controls()`` (default = 0).
	 */
	wire::id control(uint32_t const i = 0u) const;

	/*! \brief Returns the i-th target wire identifier.
	 *
	 * \param i must be less than ``num_targets()`` (default = 0).
	 */
	wire::id target(uint32_t const i = 0u) const;

	/*! \brief Returns the position in which a wire is stored in the operation. 
	 *
	 * The poistion of a qubit is unique within the operation and ``wire::id`` is a unique wire
	 * identifier within the circuit.
	 */
	uint32_t position(wire::id wire) const;

	/*! \brief Returns the wire identifier stored in the i-th position in the operation.
	 *
	 * \param i must be less than ``num_wires()``.
	 */
	wire::id wire(uint32_t const i) const;

	/*! \brief Check whether the this operation is adjoint of ``other`` operation.
	 *
	 * The conecept of _operation_ adjointness requires that operation operations to be
	 * adjoint, and that both operations act on the same qubits in the same way, i.e.,
	 * same controls and/or same targets.
	 */
	bool is_adjoint(operation const& other) const;

	/*! \brief Check whether the this operation depends on ``other`` operation.
	 *
	 * If two operations acting on the same qubits are _not_ dependent on each other, it means
	 * they can commute. For example:
	 \verbatim
	        ┌───┐                        ┌───┐
	   |0>──┤ T ├──●────       |0>────●──┤ T ├──   A T operation can comute with a CNOT 
	        └───┘  │       ──         │  └───┘     operation when the T operation it is  
	             ┌─┴─┐     ──       ┌─┴─┐          acting on the control qubit of the CNOT.
	   |0>───────┤ X ├──       |0>──┤ X ├───────
	             └───┘              └───┘  
	 \endverbatim
	 */
	bool is_dependent(operation const& other) const;
#pragma endregion

#pragma region Iterators
	/*! \brief Calls ``fn`` on every target qubit of the operation.
	 *
	 * The paramater ``fn`` is any callable that must have the following signatures:
	 * - ``void(wire::id)``
	 */
	template<typename Fn>
	void foreach_control(Fn&& fn) const;

	/*! \brief Calls ``fn`` on every target qubit of the operation.
	 *
	 * The paramater ``fn`` is any callable that must have the following signature:
	 * - ``void(wire::id)``
	 */
	template<typename Fn>
	void foreach_target(Fn&& fn) const;
#pragma endregion
};

} // namespace tweedledum