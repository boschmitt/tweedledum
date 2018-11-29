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

/*! \brief Multiple Control Single Target quantum gate.
 *
 * This class represents a gate which can act upon one, two, or three qubits of a quantum network.
 * Qubit identifier(s) 'qid' indicate on which qubit(s) the gate is acting. One qubit must be the
 * target.
 *
 * The class is composed of:
 *   - op_             : the gate operation (see enum gate_set).
 *   - target_         : indicates the slot which holds the qid of the target.
 *   - rotation_angle_ : the rotation angle of this gate.
 *   - qid_slots_      : an array which hold the qids' of the qubits this gate is acting upon.
 *                       (qids' are sorted)
 */
class mcst_gate {
public:
#pragma region Constants
	constexpr static auto max_num_qubits = 3u;
#pragma endregion

#pragma region Constructors
	/*! \brief Construct a single qubit gate
	 *
	 * \param op : the operation (must be a single qubit operation).
	 * \param qid_target : qubit identifier of the target.
	 * \param rotation_angle : it's optinal.
	 */
	mcst_gate(operation op, uint32_t qid_target, angle rotation_angle = 0.0)
	    : operation_(op)
	    , target_(0)
	    , rotation_angle_(rotation_angle)
	    , qid_slots_({qid_target, qid_invalid, qid_invalid})
	{
		assert(operation_.is_single_qubit());
		update_angle();
	}

	/*! \brief Construct a controlled gate
	 *
	 * \param op : the operation (must be a single qubit operation).
	 * \param qid_control : qubit identifier of the control.
	 * \param qid_target : qubit identifier of the target.
	 * \param rotation_angle : it's optinal.
	 */
	mcst_gate(operation op, uint32_t qid_control, uint32_t qid_target, angle rotation_angle = 0.0)
	    : operation_(op)
	    , target_(0)
	    , rotation_angle_(rotation_angle)
	    , qid_slots_({qid_target, qid_control, qid_invalid})
	{
		assert(operation_.is_double_qubit());
		assert(qid_control != qid_target);
		if (qid_control < qid_target) {
			std::swap(qid_slots_[0], qid_slots_[1]);
			target_ = 1;
		}
		update_angle();
	}

	/*! \brief Construct a gate using vectors
	 *
	 * \param op : the operation (must be a single qubit operation.
	 * \param qids_control : qubit(s) identifier of the control(s).
	 * \param qid_target : qubit identifier of the target.
	 * \param rotation_angle : it's optinal.
	 */
	mcst_gate(operation op, std::vector<uint32_t> const& qids_control,
	          std::vector<uint32_t> const& qid_target, angle rotation_angle = 0.0)
	    : operation_(op)
	    , target_(0)
	    , rotation_angle_(rotation_angle)
	    , qid_slots_({qid_invalid, qid_invalid, qid_invalid})
	{
		assert(qids_control.size() == num_controls());
		assert(qid_target.size() == 1u);
		for (auto i = 0u; i < qids_control.size(); ++i) {
			qid_slots_[i] = qids_control[i];
		}
		qid_slots_[2] = qid_target[0];
		std::sort(qid_slots_.begin(), qid_slots_.end());
		for (auto i = 0u; i < qid_slots_.size(); ++i) {
			if (qid_slots_[i] == qid_target[0]) {
				target_ = i;
				break;
			}
		}
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
		if (operation_.is_single_qubit()) {
			return 0u;
		}
		auto num_controls = 1u;
		if (operation_.is_one_of(gate_set::mcx, gate_set::mcz)) {
			++num_controls;
		}
		return num_controls;
	}

	/* !brief Return number of targets (for this gate type is always one) */
	constexpr auto num_targets() const
	{
		assert(!operation_.is_meta());
		return 1u;
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
		assert(!operation_.is_meta());
		if (operation_.is_single_qubit()) {
			return;
		}
		for (auto slot = 0u; slot < max_num_qubits; ++slot) {
			if (slot == target_ || qid_slots_[slot] == qid_invalid) {
				continue;
			}
			if constexpr (std::is_invocable_r<bool, Fn, uint32_t>::value) {
				if (!fn(qid_slots_[slot])) {
					return;
				}
			} else {
				fn(qid_slots_[slot]);
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
		fn(qid_slots_[target_]);
	}
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
	void foreach_control(Fn&& fn)
	{
		assert(!operation_.is_meta());
		if (operation_.is_single_qubit()) {
			return;
		}
		for (auto slot = 0u; slot < max_num_qubits; ++slot) {
			if (slot == target_ || qid_slots_[slot] == qid_invalid) {
				continue;
			}
			if constexpr (std::is_invocable_r<bool, Fn, uint32_t>::value) {
				if (!fn(qid_slots_[slot])) {
					return;
				}
			} else {
				fn(qid_slots_[slot]);
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
	void foreach_target(Fn&& fn)
	{
		assert(!operation_.is_meta());
		fn(qid_slots_[target_]);
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
	// 24 bytes of memory, still have 2 bytes available
	operation operation_;
	uint8_t target_;
	angle rotation_angle_;
	std::array<uint32_t, max_num_qubits> qid_slots_;
};

} // namespace tweedledum
