/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

namespace tweedledum {

template<typename Network>
using node = typename Network::node_type;

template<typename Network>
using node_ptr = typename Network::node_ptr_type;

} // namespace tweedledum
