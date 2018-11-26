/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "gate_set.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <ostream>

namespace tweedledum {

struct operation {
	gate_set value;

	constexpr operation(gate_set v)
	    : value(v)
	{}

	constexpr auto adjoint() const
	{
		return detail::gates_info[static_cast<uint8_t>(value)].adjoint;
	}

	constexpr auto is_meta() const
	{
		return (value < gate_set::identity || value == gate_set::num_defined_ops);
	}

	constexpr auto is_single_qubit() const
	{
		return (value >= gate_set::identity && value <= gate_set::t_dagger);
	}

	constexpr auto is_double_qubit() const
	{
		return (value == gate_set::cx || value == gate_set::cz);
	}

	constexpr auto is_x_rotation() const
	{
		return detail::gates_info[static_cast<uint8_t>(value)].rotation_axis == 'x';
	}

	constexpr auto is_z_rotation() const
	{
		return detail::gates_info[static_cast<uint8_t>(value)].rotation_axis == 'z';
	}

	constexpr auto is(gate_set op) const
	{
		return value == op;
	}

	template<typename... Ts>
	constexpr auto is_one_of(gate_set op) const
	{
		return is(op);
	}

	template<typename... Ts>
	constexpr auto is_one_of(gate_set t0, Ts... tn) const
	{
		return is(t0) || is_one_of(tn...);
	}

	constexpr operator gate_set() const
	{
		return value;
	}

	friend std::ostream& operator<<(std::ostream& out, operation const& op)
	{
		out << detail::gates_info[static_cast<uint8_t>(op.value)].name;
		return out;
	}
};

} // namespace tweedledum

namespace fmt {
template<>
struct formatter<tweedledum::operation> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx)
	{
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(tweedledum::operation const& op, FormatContext& ctx)
	{
		return format_to(ctx.begin(), "{}",
		                 tweedledum::detail::gates_info[static_cast<uint8_t>(op.value)].name);
	}
};
} // namespace fmt
