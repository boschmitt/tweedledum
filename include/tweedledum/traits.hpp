/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include <type_traits>

namespace tweedledum {

template<typename Network>
using node = typename Network::node_type;

template<typename Network>
using node_ptr = typename Network::node_ptr_type;

template<class Network, class = void>
struct has_allocate_qubit : std::false_type {};

template<class Network>
struct has_allocate_qubit<
    Network, std::void_t<decltype(std::declval<Network>().allocate_qubit())>>
    : std::true_type {};

template<class Network>
inline constexpr auto has_allocate_qubit_v = has_allocate_qubit<Network>::value;

template<class Network, class = void>
struct has_add_multiple_controlled_target_gate : std::false_type {};

template<class Network>
struct has_add_multiple_controlled_target_gate<Network, std::void_t<decltype(std::declval<Network>().add_multiple_controlled_target_gate(std::declval<gate_kinds_t>(), uint32_t(), uint32_t()))>> : std::true_type {};

template<class Network>
inline constexpr auto has_add_multiple_controlled_target_gate_v = has_add_multiple_controlled_target_gate<Network>::value;

} // namespace tweedledum
