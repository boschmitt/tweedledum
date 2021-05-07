/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <type_traits>
#include <utility>

namespace tweedledum {

template<class Op, class = void>
struct has_num_targets : std::false_type {};

template<class Op>
struct has_num_targets<Op,
  std::void_t<decltype(std::declval<Op>().num_targets())>> : std::true_type {};

template<class Op>
inline constexpr bool has_num_targets_v = has_num_targets<Op>::value;

//

template<class Op, class = void>
struct has_matrix : std::false_type {};

template<class Op>
struct has_matrix<Op, std::void_t<decltype(std::declval<Op>().matrix())>>
    : std::true_type {};

template<class Op>
inline constexpr bool has_matrix_v = has_matrix<Op>::value;

//

template<class Op, class = void>
struct has_adjoint : std::false_type {};

template<class Op>
struct has_adjoint<Op, std::void_t<decltype(std::declval<Op>().adjoint())>>
    : std::true_type {};

template<class Op>
inline constexpr bool has_adjoint_v = has_adjoint<Op>::value;

// Got it from: https://stackoverflow.com/a/18603716
template<class F, class... T,
  typename = decltype(std::declval<F>()(std::declval<T>()...))>
std::true_type supports_test(F const&, T const&...);
std::false_type supports_test(...);

template<class>
struct supports;

template<class F, class... T>
struct supports<F(T...)>
    : decltype(supports_test(std::declval<F>(), std::declval<T>()...)) {};

} // namespace tweedledum
