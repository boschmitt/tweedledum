/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "gates/gate_kinds.hpp"

#include <cstdint>
#include <limits>
#include <type_traits>

namespace tweedledum {

// Maybe this should not be here:
constexpr static uint32_t invalid_qid = std::numeric_limits<uint32_t>::max();

template<typename Network>
using node = typename Network::node_type;

template<typename Network>
using node_ptr = typename Network::node_ptr_type;

} // namespace tweedledum
