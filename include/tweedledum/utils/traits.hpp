/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <type_traits>

namespace tweedledum {

template<class Fn, class ElementType, class ReturnType>
constexpr auto is_callable_with_index_v = std::is_invocable_r_v<ReturnType, Fn, ElementType&, uint32_t>;

template<class Fn, class ElementType, class ReturnType>
constexpr auto is_callable_without_index_v = std::is_invocable_r_v<ReturnType, Fn, ElementType&>;

} // namespace tweedledum
