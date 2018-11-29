/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/qubit.hpp"
#include "angle.hpp"
#include "gate_set.hpp"
#include "operation.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace tweedledum {

/*! \brief Multiple Control Multiple Target reversible gate
 *
 * This class represents a gate which can act upon up to 32 qubits of a quantum network. This gate
 * type needs the network itself to have a maximum number o qubits of 32 (need to figure out how to
 * do this).
 *
 * The class is composed of:
 *   - op_             : the gate operation (see enum gate_set).
 *   - controls_       : indicates which qubits are controls.
 *   - targets_        : indicates which qubits are targets.
 *   - rotation_angle_ : the rotation angle of this gate.
 */
class mcmt_gate {
public:
#pragma region Constants
	constexpr static auto max_num_qubits = 32u;
	constexpr static auto network_max_num_qubits = 32u;
#pragma endregion

#pragma region Constructors
	/*! \brief Construct a single qubit gate
	 *
	 * \param op : the operation (must be a single qubit operation).
	 * \param qid_target : qubit identifier of the target.
	 * \param rotation_angle : it's optinal.
	 */
	mcmt_gate(operation op, uint32_t qid_target, angle rotation_angle = 0.0)
	    : operation_(op)
	    , controls_(0)
	    , targets_(0)
	    , rotation_angle_(rotation_angle)
	{
		assert(operation_.is_single_qubit());
		assert(qid_target <= network_max_num_qubits);
		targets_ |= (1 << qid_target);
		update_angle();
	}

	/*! \brief Construct a controlled gate
	 *
	 * \param op : the operation (must be a single qubit operation).
	 * \param qid_control : qubit identifier of the control.
	 * \param qid_target : qubit identifier of the target.
	 * \param rotation_angle : it's optinal.
	 */
	mcmt_gate(operation op, uint32_t qid_control, uint32_t qid_target, angle rotation_angle = 0.0)
	    : operation_(op)
	    , controls_(0)
	    , targets_(0)
	    , rotation_angle_(rotation_angle)
	{
		assert(operation_.is_double_qubit());
		assert(qid_control <= network_max_num_qubits);
		assert(qid_target <= network_max_num_qubits);
		assert(qid_control != qid_target);
		controls_ |= (1 << qid_control);
		targets_ |= (1 << qid_target);
		update_angle();
	}

	/*! \brief Construct a gate using vectors
	 *
	 * \param op : the operation (must be a single qubit operation.
	 * \param qids_control : qubit(s) identifier of the control(s).
	 * \param qid_target : qubit identifier of the target.
	 * \param rotation_angle : it's optinal.
	 */
	mcmt_gate(operation op, std::vector<uint32_t> const& qids_control,
	          std::vector<uint32_t> const& qid_target, angle rotation_angle = 0.0)
	    : operation_(op)
	    , controls_(0)
	    , targets_(0)
	    , rotation_angle_(rotation_angle)
	{
		assert(qids_control.size() <= max_num_qubits);
		assert(qid_target.size() > 0 && qid_target.size() <= max_num_qubits);
		for (auto control : qids_control) {
			assert(control <= network_max_num_qubits);
			controls_ |= (1u << control);
		}
		for (auto target : qid_target) {
			assert(target <= network_max_num_qubits);
			targets_ |= (1u << target);
		}
		assert((targets_ & controls_) == 0u);
		update_angle();
	}
#pragma endregion

#pragma region Properties
	/* !brief Return the operation performed by the gate */
	constexpr auto op() const
	{
		return operation_;
	}

	/* !brief Return number of controls */
	// cannot use 'auto' because I call this in the constructor
	uint32_t num_controls() const
	{
		assert(!operation_.is_meta());
		return __builtin_popcount(controls_);
	}

	/* !brief Return number of targets */
	constexpr auto num_targets() const
	{
		assert(!operation_.is_meta());
		return __builtin_popcount(targets_);
	}

	/* !brief Return the rotation angle of the operation perfomed by the gate */
	constexpr auto rotation_angle() const
	{
		assert(!operation_.is_meta());
		return rotation_angle_;
	}
#pragma endregion

#pragma region Const iterators
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
	void foreach_control(Fn&& fn) const
	{
		for (auto i = controls_, qid = 0u; i; i >>= 1, ++qid) {
			if (i & 1) {
				fn(qid);
			}
		}
	}

	/*! \brief Calls ``fn`` on every target qubit of the gate.
	 *
	 * The paramater ``fn`` is any callable that must have one of the following signature.
	 * - ``void(uint32_t)``
	 *
	 * ``fn`` has one parameter: a qid.
	 */
	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		assert(!operation_.is_meta());
		for (auto i = targets_, qid = 0u; i; i >>= 1, ++qid) {
			if (i & 1) {
				fn(qid);
			}
		}
	}
#pragma endregion

#pragma region Modifiers
#pragma endregion

#pragma region Overloads
#pragma endregion

#pragma region Debug
#pragma endregion

private:
	// This function updates the angle acording to the gate operation.
	constexpr void update_angle()
	{
		switch (operation_) {
		case gate_set::t:
			rotation_angle_ = symbolic_angles::one_eighth;
			break;

		case gate_set::phase:
			rotation_angle_ = symbolic_angles::one_quarter;
			break;

		case gate_set::pauli_z:
		case gate_set::cz:
		case gate_set::mcz:
		case gate_set::pauli_x:
		case gate_set::cx:
		case gate_set::mcx:
		case gate_set::hadamard:
			rotation_angle_ = symbolic_angles::one_half;
			break;

		case gate_set::phase_dagger:
			rotation_angle_ = symbolic_angles::three_fourth;
			break;

		case gate_set::t_dagger:
			rotation_angle_ = symbolic_angles::seven_eighth;
			break;

		default:
			break;
		}
	}

private:
	operation operation_;
	uint32_t controls_;
	uint32_t targets_;
	angle rotation_angle_;
};

} // namespace tweedledum
