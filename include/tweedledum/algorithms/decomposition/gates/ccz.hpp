/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../../gates/gate.hpp"
#include "../../../networks/wire.hpp"

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
void ccz(Network& network, wire::id x, wire::id y, wire::id const z)
{
	assert(!z.is_complemented());
	if (y.is_complemented() && !x.is_complemented()) {
		std::swap(x, y);
	}
	network.create_op(gate_lib::cx, y.wire(), z);
	network.create_op(x.is_complemented() ? gate_lib::t : gate_lib::tdg, z);
	network.create_op(gate_lib::cx, x.wire(), z);
	network.create_op(gate_lib::t, z);
	network.create_op(gate_lib::cx, y.wire(), z);
	network.create_op(y.is_complemented() ? gate_lib::t : gate_lib::tdg, z);
	network.create_op(gate_lib::cx, x.wire(), z);
	network.create_op(x.is_complemented() && !y.is_complemented()? gate_lib::tdg : gate_lib::t, z);
	
	network.create_op(gate_lib::cx, x.wire(), y.wire());
	network.create_op(gate_lib::tdg, y.wire());
	network.create_op(gate_lib::cx, x.wire(), y.wire());
	network.create_op(y.is_complemented() ? gate_lib::tdg : gate_lib::t, x.wire());
	network.create_op(x.is_complemented() ? gate_lib::tdg : gate_lib::t, y.wire());
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
void ccz_tpar(Network& network, wire::id x, wire::id y, wire::id const z)
{
	assert(!z.is_complemented());
	if (y.is_complemented() && !x.is_complemented()) {
		std::swap(x, y);
	}
	network.create_op(y.is_complemented() ? gate_lib::tdg : gate_lib::t, x.wire());
	network.create_op(x.is_complemented() ? gate_lib::tdg : gate_lib::t, y.wire());
	network.create_op(x.is_complemented() && !y.is_complemented() ? gate_lib::tdg : gate_lib::t, z);

	network.create_op(gate_lib::cx, x.wire(), y.wire());
	network.create_op(gate_lib::cx, y.wire(), z);
	network.create_op(gate_lib::cx, z, x.wire());

	network.create_op(x.is_complemented() ? gate_lib::t : gate_lib::tdg, x.wire());
	network.create_op(gate_lib::tdg, y.wire());
	network.create_op(gate_lib::t, z);

	network.create_op(gate_lib::cx, y.wire(), x.wire());
	network.create_op(y.is_complemented() ? gate_lib::t : gate_lib::tdg, x.wire());
	network.create_op(gate_lib::cx, y.wire(), z);
	network.create_op(gate_lib::cx, z, x.wire());
	network.create_op(gate_lib::cx, x.wire(), y.wire());
}

} // namespace tweedledum::detail