/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "ccz.hpp"
#include "../../../gates/gate.hpp"
#include "../../../networks/wire_id.hpp"

namespace tweedledum::detail {

template<typename Network>
void ccx(Network& network, wire_id x, wire_id y, wire_id z)
{
	network.add_gate(gate_lib::h, z);
	ccz_(network, x, y, z);
	network.add_gate(gate_lib::h, z);
}

template<typename Network>
void ccx_tpar(Network& network, wire_id x, wire_id y, wire_id z)
{
	network.add_gate(gate_lib::h, z);
	ccz_tpar(network, x, y, z);
	network.add_gate(gate_lib::h, z);
}

} // namespace tweedledum::detail