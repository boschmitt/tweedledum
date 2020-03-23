/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "ccz.hpp"
#include "../../../gates/gate_base.hpp"
#include "../../../gates/gate_lib.hpp"
#include "../../../networks/io_id.hpp"

namespace tweedledum::detail {

// Relative phase Tofolli gate
template<typename Network>
void rccx(Network& network, io_id x, io_id y, io_id z)
{
	network.add_gate(gate::hadamard, z);
	network.add_gate(gate::t, z);
	network.add_gate(gate::cx, y, z);
	network.add_gate(gate::t_dagger, z);
	network.add_gate(gate::cx, x, z);
	network.add_gate(gate::t, z);
	network.add_gate(gate::cx, y, z);
	network.add_gate(gate::t_dagger, z);
	network.add_gate(gate::hadamard, z);
}

} // namespace tweedledum::detail