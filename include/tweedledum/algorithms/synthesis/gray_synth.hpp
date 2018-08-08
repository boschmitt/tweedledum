/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Fereshte Mozafari
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <vector>
#include <utility>

namespace tweedledum {

/*! \brief Gray synthesis for CNOT-PHASE networks.
 *
 * This algorithm is based on the work in [M. Amy, P. Azimzadeh, M. Mosca: On
 * the complexity of CNOT-PHASE circuits, in: arXiv:1712.01859, 2017.]
 *
 * The following code shows how to apply the algorithm to the example in the
 * original paper.
 *
   \verbatim embed:rst

   .. code-block:: c++

      dag_path<qc_gate> network = ...;

      float T{0.39269908169872414}; // PI/8
      std::vector<std::pair<uint32_t, float>> parities{
          {{0b0110, T},
           {0b0001, T},
           {0b1001, T},
           {0b0111, T},
           {0b1011, T},
           {0b0011, T}}
      };
      gray_synth(network, 4, parities);
   \endverbatim
 */
template<class Network>
void gray_synth(Network& net, uint32_t nqubits,
                std::vector<std::pair<uint32_t, float>> const& parities)
{
	// ...
}

} // namespace tweedledum
