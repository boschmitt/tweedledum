/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../networks/netlist.hpp"
#include "../../traits.hpp"
#include "single_target_gates.hpp"

#include <cstdint>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/esop.hpp>
#include <vector>

namespace tweedledum {

/*! \brief Control function synthesis algorithm.
 *
 * This function synthesizes a circuit from an _n_-variable Boolean function.
 * The resulting circuit has _n+1_ qubits, where the first _n_ qubits hold the
 * input to the Boolean function (and remain unchanged) and the last qubit
 * computes the output of the function by XOR-ing it to its current value.
 *
   \verbatim embed:rst

   The following code shows how to apply the algorithm to the majority-of-five
   function.

   .. code-block:: c++

      kitty::dynamic_truth_table tt(5);
      kitty::create_majority(tt);
      auto circ = control_function_synthesis<gg_network<mcmt_gate>>(tt);
   \endverbatim
 *
 * \param tt A truth table (see <a href="http://github.com/msoeken/kitty">
 *           kitty</a>)
 * \param stg_fn A synthesis function for single-target gates
 *
 * \algtype synthesis
 * \algexpects Truth table
 * \algreturns Quantum or reversible circuit
 */
template<typename Network, typename SingleTargetGateSynthesisFn = stg_from_pprm>
Network control_function_synthesis(kitty::dynamic_truth_table const& tt,
                                   SingleTargetGateSynthesisFn&& stg_fn = {})
{
	Network circ;
	const uint32_t num_qubits = tt.num_vars() + 1;
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.add_qubit();
	}

	std::vector<uint32_t> qubit_map(circ.num_qubits());
	std::iota(qubit_map.begin(), qubit_map.end(), 0u);
	stg_fn(circ, tt, qubit_map);
	return circ;
}

}; // namespace tweedledum
