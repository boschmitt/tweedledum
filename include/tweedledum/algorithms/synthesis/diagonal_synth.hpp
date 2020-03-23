/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/wire_id.hpp"
#include "../../utils/angle.hpp"
#include "../../utils/parity_terms.hpp"
#include "gray_synth.hpp"

#include <algorithm>
#include <vector>

namespace tweedledum {

namespace detail {

inline void fast_hadamard_transform(std::vector<angle>& angles)
{
	uint32_t k = 0u;
	for (uint32_t m = 1u; m < angles.size(); m <<= 1u) {
		for (uint32_t i = 0u; i < angles.size(); i += (m << 1u)) {
			for (uint32_t j = i, p = k = i + m; j < p; ++j, ++k) {
				angle t = angles.at(j);
				angles.at(j) += angles.at(k);
				angles.at(k) = t - angles.at(k);
			}
		}
	}
}

} // namespace detail

/*! \brief Synthesis of diagonal unitary matrices.
 *
 * This is the in-place variant of ``diagonal_synth``, in which the network is passed as a parameter
 * and can potentially already contain some gates.  The parameter ``qubits`` provides a qubit
 * mapping to the existing qubits in the network.
 *
 * \param network A quantum network
 * \param qubits The subset of qubits the linear reversible circuit acts upon
 * \param angles Angles for diagonal matrix elements
 */
template<class Network>
void diagonal_synth(Network& network, std::vector<wire_id> const& qubits,
                    std::vector<angle> const& angles)
{
	// Number of angles + 1 needs to be a power of two!
	assert(!angles.empty() && !((angles.size() + 1) & angles.size()));
	assert(!qubits.empty() && qubits.size() <= 32);
	assert((1u << qubits.size()) == (angles.size() + 1u));

	// Normalize input angles
	std::vector<angle> norm_angles;
	norm_angles.emplace_back(sym_angle::zero);
	std::transform(angles.begin(), angles.end(), std::back_inserter(norm_angles),
	               [](angle a) { return -a; });

	detail::fast_hadamard_transform(norm_angles);
	uint32_t factor = (1 << (qubits.size() - 1));
	parity_terms<uint32_t> parities;
	for (uint32_t i = 1u; i < norm_angles.size(); ++i) {
		if (norm_angles.at(i) == sym_angle::zero) {
			continue;
		}
		parities.add_term(i, norm_angles.at(i) / factor);
	}
	gray_synth(network, qubits, parities);
}

/*! \brief Synthesis of diagonal unitary matrices.
 *
   \verbatim embed:rst

   This algorithm is based on the work in :cite:`SS03`.  It takes as input :math:`2^n - 1` real
   angles :math:`\theta_i` for :math:`i = 1, \dots, 2^{n-1}` and returns a quantum circuit that
   realizes the :math:`2^n \times 2^n` unitary operation

   :math:`U = \mathrm{diag}(1, e^{i\theta_1}, e^{i\theta_2}, \dots, e^{i\theta_{2^{n-1}}}).`

   It uses a fast Hadamard transformation to compute angles for parity terms that are passed to the
   Gray synthesis algorithm ``gray_synth``.
   
   \endverbatim
 * \param angles Angles for diagonal matrix elements
 * \return {CNOT, Rz} network
 *
 * \algtype synthesis
 * \algexpects List of angles in diagonal unitary matrix
 * \algreturns {CNOT, Rz} network
 */
template<class Network>
Network diagonal_synth(std::vector<angle> const& angles)
{
	// Number of angles + 1 needs to be a power of two!
	assert(!angles.empty() && !((angles.size() + 1) & angles.size()));
	uint32_t num_qubits = __builtin_ctz(angles.size() + 1);
	assert(num_qubits <= 32u);

	Network network;
	std::vector<wire_id> qubits;
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		qubits.emplace_back(network.create_qubit());
	}
	diagonal_synth(network, qubits, angles);
	return network;
}

} // namespace tweedledum
