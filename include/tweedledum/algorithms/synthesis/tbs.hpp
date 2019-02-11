/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt, Mathias Soeken
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/qubit.hpp"
#include "../../networks/netlist.hpp"

#include <cmath>
#include <cstdint>
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <kitty/detail/mscfix.hpp>
#include <list>
#include <numeric>
#include <vector>

namespace tweedledum {

/*! \brief Parameters for `tbs`. */
struct tbs_params {
	/*! \brief Variant of transformation-based synthesis.
	   \verbatim embed:rst
	   * Unidirectional: only adds gates from the output side
	   * Bidirectional: adds gates from input or output side at each step
		 * Multidirectional: adds gates from input and output side at each step (see :cite:`SDRM16`)
	   \endverbatim
	 */
	enum class behavior : uint8_t {
		unidirectional,
		bidirectional,
		multidirectional,
	} behavior = behavior::unidirectional;

	using cost_fn_type = std::function<uint32_t(std::vector<uint32_t> const&, uint32_t, uint32_t)>;
	/*! \brief Cost function in multi-directional synthesis.
	 *
	 * By default the number of reversible gates is used as cost function.
	 */
	cost_fn_type cost_fn = [](auto& perm, auto x, auto z) {
		// hamming distance from z to x and from x to f(z)
		return __builtin_popcount(z ^ x) + __builtin_popcount(x ^ perm[z]);
	};

	/*! \brief Be verbose. */
	bool verbose = false;
};

namespace detail {

// TODO: maybe take it out from here (put it in 'utils')
template<class IntType>
inline std::vector<qubit_id> to_qubit_vector(IntType bits, std::vector<qubit_id> const& qubits)
{
	std::vector<qubit_id> ret;
	auto index = 0u;
	while (bits) {
		if (bits & 1) {
			ret.push_back(qubits.at(index));
		}
		bits >>= 1;
		++index;
	}
	return ret;
}

inline void update_permutation(std::vector<uint32_t>& perm, uint32_t controls, uint32_t targets)
{
	std::for_each(perm.begin(), perm.end(), [&](auto& z) {
		if ((z & controls) == controls) {
			z ^= targets;
		}
	});
}

inline void update_permutation_inv(std::vector<uint32_t>& perm, uint32_t controls, uint32_t targets)
{
	for (auto i = 0u; i < perm.size(); ++i) {
		if ((i & controls) != controls) {
			continue;
		}
		if (const auto partner = i ^ targets; partner > i) {
			std::swap(perm[i], perm[partner]);
		}
	}
}

template<typename Network>
void tbs_unidirectional(Network& network, std::vector<qubit_id> const& qubits,
                        std::vector<uint32_t>& perm, tbs_params params = {})
{
	std::vector<std::pair<uint32_t, uint32_t>> gates;
	for (auto x = 0u; x < perm.size(); ++x) {
		// skip identity lines
		if (perm[x] == x) {
			continue;
		}

		auto y = perm[x];
		// move 0s to 1s
		if (const uint32_t t01 = x & ~y) {
			detail::update_permutation(perm, y, t01);
			gates.emplace_back(y, t01);
		}

		// move 1s to 0s
		if (const uint32_t t10 = ~x & y) {
			detail::update_permutation(perm, x, t10);
			gates.emplace_back(x, t10);
		}
	}
	std::reverse(gates.begin(), gates.end());
	for (const auto [controls, targets] : gates) {
		network.add_gate(gate::mcx, detail::to_qubit_vector(controls, qubits),
		                 detail::to_qubit_vector(targets, qubits));
	}
}

template<typename Network>
void tbs_bidirectional(Network& network, std::vector<qubit_id> const& qubits,
                       std::vector<uint32_t>& perm, tbs_params params = {})
{
	std::list<std::pair<uint32_t, uint32_t>> gates;
	auto pos = gates.begin();
	for (auto x = 0u; x < perm.size(); ++x) {
		// skip identity lines
		if (perm[x] == x) {
			continue;
		}

		auto y = perm[x];
		const uint32_t xs = std::distance(perm.begin(), std::find(perm.begin() + x, perm.end(), x));
		if (__builtin_popcount(x ^ y) <= __builtin_popcount(x ^ xs)) {
			/* move 0s to 1s */
			if (const uint32_t t01 = x & ~y) {
				detail::update_permutation(perm, y, t01);
				pos = gates.emplace(pos, y, t01);
			}
			/* move 1s to 0s */
			if (const uint32_t t10 = ~x & y) {
				detail::update_permutation(perm, x, t10);
				pos = gates.emplace(pos, x, t10);
			}
		} else {
			/* move 0s to 1s */
			if (const uint32_t t01 = ~xs & x) {
				detail::update_permutation_inv(perm, xs, t01);
				pos = gates.emplace(pos, xs, t01);
				pos++;
			}
			/* move 1s to 0s */
			if (const uint32_t t10 = xs & ~x) {
				detail::update_permutation_inv(perm, x, t10);
				pos = gates.emplace(pos, x, t10);
				pos++;
			}
		}
	}
	for (const auto [controls, targets] : gates) {
		network.add_gate(gate::mcx, detail::to_qubit_vector(controls, qubits),
		                 detail::to_qubit_vector(targets, qubits));
	}
}

template<typename Network>
void tbs_multidirectional(Network& network, std::vector<qubit_id> const& qubits,
                          std::vector<uint32_t>& perm, tbs_params params = {})
{
	std::list<std::pair<uint32_t, uint32_t>> gates;
	auto pos = gates.begin();
	for (auto x = 0u; x < perm.size(); ++x) {
		// find cheapest assignment
		auto x_best = x;
		uint32_t x_cost = __builtin_popcount(x ^ perm[x]);
		for (auto xx = x + 1; xx < perm.size(); ++xx) {
			if (const auto cost = params.cost_fn(perm, x, xx); cost < x_cost) {
				x_best = xx;
				x_cost = cost;
			}
		}

		const auto z = x_best;
		const auto y = perm[z];

		// map z |-> x
		// move 0s to 1s
		if (const uint32_t t01 = ~z & x) {
			detail::update_permutation_inv(perm, z, t01);
			pos = gates.emplace(pos, z, t01);
			pos++;
		}

		// move 1s to 0s
		if (const uint32_t t10 = z & ~x) {
			detail::update_permutation_inv(perm, x, t10);
			pos = gates.emplace(pos, x, t10);
			pos++;
		}

		// map y |-> x
		// move 0s to 1s
		if (const uint32_t t01 = x & ~y) {
			detail::update_permutation(perm, y, t01);
			pos = gates.emplace(pos, y, t01);
		}

		// move 1s to 0s
		if (const uint32_t t10 = ~x & y) {
			detail::update_permutation(perm, x, t10);
			pos = gates.emplace(pos, x, t10);
		}
	}
	for (const auto [controls, targets] : gates) {
		network.add_gate(gate::mcx, detail::to_qubit_vector(controls, qubits),
		                 detail::to_qubit_vector(targets, qubits));
	}
}

} // namespace detail

/*! \brief Transformation-based reversible logic synthesis.
 *
 * This is the in-place variant of ``tbs``, in which the network is passed as
 * a parameter and can potentially already contain some gates.  The parameter
 * ``qubits`` provides a qubit mapping to the existing qubits in the network.
 *
 * \param network A quantum circuit
 * \param qubits A qubit mapping
 * \param perm A permutation
 * \param params Parameters (see ``tbs_params``)
 */
template<typename Network>
void tbs(Network& network, std::vector<qubit_id> const& qubits, std::vector<uint32_t> perm,
         tbs_params params = {})
{
	assert(network.num_qubits() >= qubits.size());

	switch (params.behavior) {
		case tbs_params::behavior::unidirectional:
			detail::tbs_unidirectional(network, qubits, perm, params);
			break;
		case tbs_params::behavior::bidirectional:
			detail::tbs_bidirectional(network, qubits, perm, params);
			break;
		case tbs_params::behavior::multidirectional:
			detail::tbs_multidirectional(network, qubits, perm, params);
			break;
	}
}

/*! \brief Transformation-based reversible logic synthesis.
 *
   \verbatim embed:rst
   This algorithm implements various variants of the transformation-based
   synthesis algorithm originally proposed in :cite:`MMD03`.
   A permutation is specified as a vector of :math:`2^n` different integers ranging from :math:`0`
   to :math:`2^n-1`.

   .. code-block:: c++

      std::vector<uint32_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
      auto network = tbs<netlist<mcst_gate>>(permutation);

   \endverbatim
 *
 * \param perm A permutation
 * \param params Parameters (see ``tbs_params``)
 * 
 * \algtype synthesis
 * \algexpects Permutation
 * \algreturns Reversible circuit
 */
template<typename Network>
Network tbs(std::vector<uint32_t> perm, tbs_params params = {})
{
	Network network;
	const uint32_t num_qubits = std::log2(perm.size());
	for (auto i = 0u; i < num_qubits; ++i) {
		network.add_qubit();
	}

	std::vector<qubit_id> qubits(num_qubits);
	std::iota(qubits.begin(), qubits.end(), 0u);
	tbs(network, qubits, perm, params);
	return network;
}

}; // namespace tweedledum
