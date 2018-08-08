/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <vector>

namespace tweedledum {

/*! \brief Linear circuit synthesis
 *
 * This algorithm is based on the work in [K.N. Patel, I.L. Markov, J.P. Hayes:
 * Optimal synthesis of linear reversible circuits, in QIC 8, 3&4, 282-294,
 * 2008.]
 *
 * The following code shows how to apply the algorithm to the example in the
 * original paper.
 *
   \verbatim embed:rst

   .. code-block:: c++

      dag_path<qc_gate> network = ...;

      std::vector<uint32_t> matrix{{0b110000,
                                    0b100110,
                                    0b010010,
                                    0b111111,
                                    0b110111,
                                    001110}};
      cnot_patel(network, matrix, 2);
   \endverbatim
 */
template<class Network>
void cnot_patel(Network& net, std::vector<uint32_t> const& matrix,
                uint32_t partition_size)
{
	/* number of qubits can be taken from matrix, since it is n x n matrix. */
	const auto nqubits = matrix.size();

	// ...
}

} // namespace tweedledum
