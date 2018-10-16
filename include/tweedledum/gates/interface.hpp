/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "gate_kinds.hpp"

#include <string>
#include <vector>

namespace tweedledum {

static_assert(false,
              "file gates/interface.hpp cannot be included, it's only used for documentation");

/*! \brief Type representing a gate. */
class gate final {
public:
#pragma region Constants
	static constexpr uint32_t max_num_qubits;
	static constexpr uint32_t network_max_num_qubits;
#pragma endregion

#pragma region Constructors
	gate(gate_kinds_t kind, uint32_t target);
	gate(gate_kinds_t kind, uint32_t control, uint32_t target);
	gate(gate_kinds_t kind, std::vector<uint32_t> controls, std::vector<uint32_t> targets);
#pragma endregion

#pragma region Properties
	/*! \brief Returns the number of controls. */
	auto num_controls() const;

	/*! \brief Returns the number of targets. */
	auto num_targets() const;

	/*! \brief Returns the gate kind. */
	auto kind() const;

	/*! \brief Returns true if gate kind is ``k``. */
	bool is(gate_kinds_t k) const;

	/*! \brief Returns true if gate kind is one of ``(k0 ... kn)``. */
	template<typename... Ks>
	bool is_one_of(gate_kinds_t k0, Ks... kn) const;

	/*! \brief Check whether the this gate depends on ``other`` gate.
	 *
	 * If two gates acting on the same qubits are _not_ dependent on each
	 * other, it means they can commute. For example:
	 *
	 *  -- T --o--     -- o -- T --    A T gate can comute with a CNOT gate
	 *         |    =     |            when the T gate it is acting on the
	 *  ------ x -     -- x -------    control qubit of the CNOT.
	 */
	bool is_dependent(gate const& other) const;

	/*! \brief Checks weather a qubit is a control for the gate */
	auto is_control(uint32_t qubit_id) const;

	/*! \brief Returns the angle in case of rotation gate. */
	auto rotation_angle() const;

	/*! \brief Returns the index of a qubit in the gate.
	 *
	 * The index of a qubit is unique within the gate and ``qubit_id`` is
	 * a unique qubit identifier within the circuit.
	 */
	auto qubit_index(uint32_t qubit_id) const;
#pragma endregion

#pragma region Iterators
	/*! \brief Calls ``fn`` on every target qubit of the gate.
	 *
	 * The paramater ``fn`` is any callable that must have
	 * one of the following four signatures.
	 * - ``void(uint32_t)``
	 * - ``bool(uint32_t)``
	 *
	 * If ``fn`` has one parameter: a qubit id. If ``fn`` returns a ``bool``,
	 * then it can interrupt the iteration by returning ``false``.
	 */
	template<typename Fn>
	void foreach_target(Fn&& fn) const;

	/*! \brief Calls ``fn`` on every control qubit of the gate.
	 *
	 * The paramater ``fn`` is any callable that must have
	 * one of the following four signatures.
	 * - ``void(uint32_t)``
	 * - ``bool(uint32_t)``
	 *
	 * If ``fn`` has one parameter: a qubit id. If ``fn`` returns a ``bool``,
	 * then it can interrupt the iteration by returning ``false``.
	 */
	template<typename Fn>
	void foreach_control(Fn&& fn) const;
#pragma endregion
};

} // namespace tweedledum
