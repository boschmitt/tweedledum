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

struct mct_gate {
	static constexpr uint32_t max_num_qubits = 32;

	uint32_t controls{};
	uint32_t targets{};

	auto num_controls() const
	{
		return __builtin_popcount(controls);
	}

	auto num_targets() const
	{
		return __builtin_popcount(targets);
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
		return kind == gate_kinds_t::mcx;
	}

	gate_kinds_t kind() const
	{
		return gate_kinds_t::mcx;
	}
};

}; // namespace tweedledum
