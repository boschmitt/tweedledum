/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "gate_kinds.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <list>
#include <stack>
#include <vector>

namespace tweedledum {

struct pmct_gate {
	static constexpr uint32_t max_num_qubits = 32;

	uint32_t controls{};
	uint32_t targets{};
	uint32_t kind_ = static_cast<uint32_t>(gate_kinds_t::mcx);

	auto num_controls() const
	{
		return __builtin_popcount(controls);
	}

	auto num_targets() const
	{
		return __builtin_popcount(targets);
	}

	auto angle() const
	{
		return 0;
	}

	template<typename Fn>
	void foreach_control(Fn&& fn) const
	{
		for (auto i = controls, j = 0u; i; i >>= 1, ++j) {
			if (i & 1) {
				fn(j);
			}
		}
	}

	template<typename Fn>
	void foreach_target(Fn&& fn) const
	{
		for (auto i = targets, j = 0u; i; i >>= 1, ++j) {
			if (i & 1) {
				fn(j);
			}
		}
	}

	bool is(gate_kinds_t kind) const
	{
		return static_cast<uint32_t>(kind) == kind_;
	}

	void kind(gate_kinds_t kind)
	{
		kind_ = static_cast<uint32_t>(kind);
	}

	gate_kinds_t kind() const
	{
		return static_cast<gate_kinds_t>(kind_);
	}
};

}; // namespace tweedledum
