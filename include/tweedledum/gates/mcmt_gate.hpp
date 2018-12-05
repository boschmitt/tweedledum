/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../networks/qubit.hpp"
#include "gate_set.hpp"
#include "gate_base.hpp"

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
	mcmt_gate(gate_base const& op, uint32_t qid_target)
	    : gate_base(op)
	    , controls_(0)
	    , targets_(0)
	{
		assert(is_single_qubit());
		assert(qid_target <= network_max_num_qubits);
		targets_ |= (1 << qid_target);
	}

	mcmt_gate(gate_base const& op, uint32_t qid_control, uint32_t qid_target)
	    : gate_base(op)
	    , controls_(0)
	    , targets_(0)
	{
		assert(is_double_qubit());
		assert(qid_control <= network_max_num_qubits);
		assert(qid_target <= network_max_num_qubits);
		assert(qid_control != qid_target);
		controls_ |= (1 << qid_control);
		targets_ |= (1 << qid_target);
	}

	mcmt_gate(gate_base const& op, std::vector<uint32_t> const& qids_control,
	          std::vector<uint32_t> const& qid_target)
	    : gate_base(op)
	    , controls_(0)
	    , targets_(0)
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
		for (auto i = controls_, qid = 0u; i; i >>= 1, ++qid) {
			if (i & 1) {
				fn(qid);
			}
		}
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		assert(!is_meta());
		for (auto i = targets_, qid = 0u; i; i >>= 1, ++qid) {
			if (i & 1) {
				fn(qid);
			}
		}
	}
#pragma endregion

private:
	/*! \brief bitmap which indicates which qubits in the network are the controls. */ 
	uint32_t controls_;
	/*! \brief bitmap which indicates which qubits in the network are the targets. */ 
	uint32_t targets_;
};

} // namespace tweedledum
