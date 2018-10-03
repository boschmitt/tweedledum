/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*------------------------------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <type_traits>

/* Taken from mockturtle (: */

namespace tweedledum::detail {

template<class Fn, class ElementType, class ReturnType>
inline constexpr bool is_callable_with_index_v
    = std::is_invocable_r_v<ReturnType, Fn, ElementType&, std::uint32_t>;

template<class Fn, class ElementType, class ReturnType>
inline constexpr bool is_callable_without_index_v = std::is_invocable_r_v<ReturnType, Fn, ElementType&>;

template<class Iterator, class ElementType = typename Iterator::value_type, class Fn>
Iterator foreach_element(Iterator begin, Iterator end, Fn&& fn, std::uint32_t counter_offset = 0)
{
	// clang-format off
	static_assert(is_callable_with_index_v<Fn, ElementType&, void> ||
	              is_callable_without_index_v<Fn, ElementType&, void> ||
		      is_callable_with_index_v<Fn, ElementType&, bool> ||
		      is_callable_without_index_v<Fn, ElementType&, bool>);
	// clang-format on

	if constexpr (is_callable_without_index_v<Fn, ElementType&, bool>) {
		(void) counter_offset;
		while (begin != end) {
			if (!fn(*begin++)) {
				return begin;
			}
		}
		return begin;
	} else if constexpr (is_callable_with_index_v<Fn, ElementType&, bool>) {
		std::uint32_t index{counter_offset};
		while (begin != end) {
			if (!fn(*begin++, index++)) {
				return begin;
			}
		}
		return begin;
	} else if constexpr (is_callable_without_index_v<Fn, ElementType&, void>) {
		(void) counter_offset;
		while (begin != end) {
			fn(*begin++);
		}
		return begin;
	} else if constexpr (is_callable_with_index_v<Fn, ElementType&, void>) {
		std::uint32_t index{counter_offset};
		while (begin != end) {
			fn(*begin++, index++);
		}
		return begin;
	}
}

} // namespace tweedledum::detail
