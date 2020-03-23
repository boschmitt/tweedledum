/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../gates/gate.hpp"
#include "../../../networks/wire_id.hpp"

namespace tweedledum::detail {

/*                                                           ┌───┐
 *  ──────────────●───────────────────●─────────●─────────●──┤ T ├
 *                │                   │         │         │  └───┘
 *                │                   │       ┌─┴─┐┌───┐┌─┴─┐┌───┐
 *  ────●─────────┼─────────●─────────┼───────┤ X ├┤ ┴ ├┤ X ├┤ T ├
 *      │         │         │         │       └───┘└───┘└───┘└───┘
 *    ┌─┴─┐┌───┐┌─┴─┐┌───┐┌─┴─┐┌───┐┌─┴─┐                    ┌───┐
 *  ──┤ X ├┤ ┴ ├┤ X ├┤ T ├┤ X ├┤ ┴ ├┤ X ├────────────────────┤ T ├
 *    └───┘└───┘└───┘└───┘└───┘└───┘└───┘                    └───┘
 *
 * NOTE: Important normalization: if only one control is complemented, it must be x. This means
 * that a complemented `y` implies (->) a complemented `x`.
 */
template<typename Network>
void ccz(Network& network, wire_id x, wire_id y, wire_id z)
{
	assert(!y.is_complemented() || x.is_complemented());
	assert(!z.is_complemented());
	network.create_op(gate_lib::cx, y.id(), z);
	network.create_op(x.is_complemented() ? gate_lib::t : gate_lib::tdg, z);
	network.create_op(gate_lib::cx, x.id(), z);
	network.create_op(gate_lib::t, z);
	network.create_op(gate_lib::cx, y.id(), z);
	network.create_op(y.is_complemented() ? gate_lib::t : gate_lib::tdg, z);
	network.create_op(gate_lib::cx, x.id(), z);
	network.create_op(x.is_complemented() && !y.is_complemented()? gate_lib::tdg : gate_lib::t, z);
	
	network.create_op(gate_lib::cx, x.id(), y);
	network.create_op(gate_lib::tdg, y);
	network.create_op(gate_lib::cx, x.id(), y);
	network.create_op(y.is_complemented() ? gate_lib::tdg : gate_lib::t, x);
	network.create_op(x.is_complemented() ? gate_lib::tdg : gate_lib::t, y);
}

/* Better T gate parallelization at the expense of an extra CNOT gate.
 * 
 *   ┌───┐          ┌───┐┌───┐┌───┐┌───┐     ┌───┐     
 * ──┤ T ├──●───────┤ X ├┤ ┴ ├┤ X ├┤ ┴ ├─────┤ X ├──●──
 *   └───┘  │       └─┬─┘└───┘└─┬─┘└───┘     └─┬─┘  │  
 *   ┌───┐┌─┴─┐       │  ┌───┐  │              │  ┌─┴─┐
 * ──┤ T ├┤ X ├──●────┼──┤ ┴ ├──●─────────●────┼──┤ X ├
 *   └───┘└───┘  │    │  └───┘            │    │  └───┘
 *   ┌───┐     ┌─┴─┐  │  ┌───┐          ┌─┴─┐  │       
 * ──┤ T ├─────┤ X ├──●──┤ T ├──────────┤ X ├──●───────
 *   └───┘     └───┘     └───┘          └───┘          
 *
 * NOTE: Important normalization: if only one control is complemented, it must be x. This means
 * that a complemented `y` implies (->) a complemented `x`.
 */
template<typename Network>
void ccz_tpar(Network& network, wire_id x, wire_id y, wire_id z)
{
	assert(!y.is_complemented() || x.is_complemented());
	assert(!z.is_complemented());
	network.create_op(y.is_complemented() ? gate_lib::tdg : gate_lib::t, x.id());
	network.create_op(x.is_complemented() ? gate_lib::tdg : gate_lib::t, y.id());
	network.create_op(x.is_complemented() && !y.is_complemented() ? gate_lib::tdg : gate_lib::t, z);

	network.create_op(gate_lib::cx, x.id(), y.id());
	network.create_op(gate_lib::cx, y.id(), z);
	network.create_op(gate_lib::cx, z, x.id());

	network.create_op(x.is_complemented() ? gate_lib::t : gate_lib::tdg, x);
	network.create_op(gate_lib::tdg, y.id());
	network.create_op(gate_lib::t, z);

	network.create_op(gate_lib::cx, y.id(), x.id());
	network.create_op(y.is_complemented() ? gate_lib::t : gate_lib::tdg, x);
	network.create_op(gate_lib::cx, y.id(), z);
	network.create_op(gate_lib::cx, z, x.id());
	network.create_op(gate_lib::cx, x.id(), y.id());
}

} // namespace tweedledum::detail