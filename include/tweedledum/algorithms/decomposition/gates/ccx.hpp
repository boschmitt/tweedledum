/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "ccz.hpp"
#include "../../../gates/gate_base.hpp"
#include "../../../gates/gate_lib.hpp"
#include "../../../networks/io_id.hpp"

#include <array>
#include <vector>

namespace tweedledum::detail {

template<typename Network>
void ccx_(Network& network, io_id x, io_id y, io_id z)
{
	network.add_gate(gate::hadamard, z);
	ccz_(network, x, y, z);
	network.add_gate(gate::hadamard, z);
}

template<typename Network>
void ccx_tpar(Network& network, io_id x, io_id y, io_id z)
{
	network.add_gate(gate::hadamard, z);
	ccz_tpar(network, x, y, z);
	network.add_gate(gate::hadamard, z);
}

template<typename Network>
void ccx(Network& network, io_id x, io_id y, io_id z, bool use_t_par)
{
	if (use_t_par) {
		ccx_tpar(network, x, y, z);
	} else {
		ccx_(network, x, y, z);
	}
}

} // namespace tweedledum::detail