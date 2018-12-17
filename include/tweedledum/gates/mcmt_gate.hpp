/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/qubit.hpp"
#include "gate_base.hpp"
#include "gate_set.hpp"

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
 */
class mcmt_gate final : public gate_base {
public:
#pragma region Constants
	constexpr static auto max_num_qubits = 32u;
	constexpr static auto network_max_num_qubits = 32u;
#pragma endregion

#pragma region Constructors
	mcmt_gate(gate_base const& op, qubit_id target)
	    : gate_base(op)
	    , controls_(0)
	    , polarity_(0)
	    , targets_(0)
	{
		assert(is_single_qubit());
		assert(target <= network_max_num_qubits);
		targets_ |= (1 << target);
	}

	mcmt_gate(gate_base const& op, qubit_id control, qubit_id target)
	    : gate_base(op)
	    , controls_(0)
	    , polarity_(0)
	    , targets_(0)
	{
		assert(is_double_qubit());
		assert(control <= network_max_num_qubits);
		assert(target <= network_max_num_qubits);
		assert(control != target);
		controls_ |= (1 << control);
		targets_ |= (1 << target);
		polarity_ |= (control.is_complemented() << control.index());
	}

	mcmt_gate(gate_base const& op, std::vector<qubit_id> const& controls,
	          std::vector<qubit_id> const& target)
	    : gate_base(op)
	    , controls_(0)
	    , polarity_(0)
	    , targets_(0)
	{
		assert(controls.size() <= max_num_qubits);
		assert(target.size() > 0 && target.size() <= max_num_qubits);
		for (auto control : controls) {
			assert(control <= network_max_num_qubits);
			controls_ |= (1u << control);
			polarity_ |= (control.is_complemented() << control.index());
		}
		for (auto target : target) {
			assert(target <= network_max_num_qubits);
			targets_ |= (1u << target);
		}
		assert((targets_ & controls_) == 0u);
	}
#pragma endregion

#pragma region Properties
	uint32_t num_controls() const
	{
		assert(!is_meta());
		return __builtin_popcount(controls_);
	}

	constexpr auto num_targets() const
	{
		assert(!is_meta());
		return __builtin_popcount(targets_);
	}
#pragma endregion

#pragma region Const iterators
	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		for (auto i = controls_, id = 0u, p = polarity_; i; i >>= 1, ++id, p >>= 1) {
			if (i & 1) {
				fn(qubit_id(id, (p & 1)));
			}
		}
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		assert(!is_meta());
		for (auto i = targets_, id = 0u; i; i >>= 1, ++id) {
			if (i & 1) {
				fn(qubit_id(id, 0));
			}
		}
	}
#pragma endregion

private:
	/*! \brief bitmap which indicates which qubits in the network are the controls. */
	uint32_t controls_;
	/*! \brief bitmap which indicates the controls' polarities. */
	uint32_t polarity_;
	/*! \brief bitmap which indicates which qubits in the network are the targets. */
	uint32_t targets_;
};

} // namespace tweedledum
