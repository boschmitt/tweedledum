/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include <fmt/format.h>

namespace tweedledum {

enum class gate_set : uint8_t {
#define GATE(X, Y, Z, W) X,
#include "gate_set.def"
	num_defined_ops,
};

namespace detail {

struct table_entry {
	gate_set adjoint;
	uint8_t rotation_axis;
	char const* name;
};

constexpr table_entry gates_info[] = {
#define GATE(X, Y, Z, W) {gate_set::Y, Z, #W},
#include "gate_set.def"
    {gate_set::undefined, '-', "Error"},
};

} // namespace detail
} // namespace tweedledum
