/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"


// This implementation is based on:
//
// Miller, D. Michael, Dmitri Maslov, and Gerhard W. Dueck. "A transformation
// based algorithm for reversible logic synthesis." Proceedings 2003. design
// automation conference (ieee cat. no. 03ch37451). IEEE, 2003.
//
// Starting from a reversible function, transformation-based synthesis applies
// gates and adjusts the function representation accordingly in a way that each
// gate application gets the function closer to the identity function.  If the
// identity function has been reached, all applied gates make up for the circuit
// that realizes the initial function.
//
// Here there is also the implementation of a multidirectional method based on:
//
// Soeken, Mathias, Gerhard W. Dueck, and D. Michael Miller. "A fast symbolic
// transformation based algorithm for reversible logic synthesis." International
// Conference on Reversible Computation. Springer, Cham, 2016.
//
// Variants:
// (*) unidirectional: only adds gates from the output side
// (*) bidirectional: adds gates from input __or__ output side at each step
// (*) multidirectional: adds gates from input __and__ output side at each step
// 
namespace tweedledum {
#pragma region Implementation details
namespace transform_synth_detail {

using AbstractGate = std::pair<uint32_t, uint32_t>;
using GateList = std::vector<AbstractGate>;

inline void update_permutation(
    std::vector<uint32_t>& perm, uint32_t controls, uint32_t targets)
{
	for (uint32_t i = 0; i < perm.size(); ++i) {
		if ((perm[i] & controls) == controls) {
			perm[i] ^= targets;
		}
	}
}

inline GateList unidirectional(std::vector<uint32_t> perm)
{
	GateList gates;
	for (uint32_t i = 0u; i < perm.size(); ++i) {
		// Skip identity lines
		if (perm[i] == i) {
			continue;
		}
		uint32_t const y = perm[i];
		// Let p be the bit string with 1's in all positions where the
		// binary expansion of i is 1 while the expansion of perm[i] is
		// 0.
		uint32_t const p = i & ~y;
		// For each pj = 1, apply the Toffoli gate with control lines
		// corresponding to all outputs in positions where the expansion
		// of a is 1 and whose target line is the output in position j.
		if (p) {
			update_permutation(perm, y, p);
			gates.emplace_back(y, p);
		}

		// Lett q be the bit string with 1's in all positions where the
		// expansion of i is 0 while the expansion of perm[i] is 1.
		uint32_t const q = ~i & y;
		// For each qk = 1, apply the Toffoli gate with control lines
		// corresponding to all outputs in positions where the expansion
		// of f+(i) is 1 and whose target line is the output in k.
		if (q) {
			update_permutation(perm, i, q);
			gates.emplace_back(i, q);
		}
	}
	std::reverse(gates.begin(), gates.end());
	return gates;
}

inline void update_permutation_inv(
    std::vector<uint32_t>& perm, uint32_t controls, uint32_t targets)
{
	for (uint32_t i = 0u; i < perm.size(); ++i) {
		if ((i & controls) != controls) {
			continue;
		}
		uint32_t const partner = i ^ targets;
		if ( partner > i) {
			std::swap(perm[i], perm[partner]);
		}
	}
}

inline GateList bidirectional(std::vector<uint32_t> perm)
{
	GateList gates;
	auto pos = gates.begin();
	for (uint32_t i = 0u; i < perm.size(); ++i) {
		if (perm[i] == i) {
			continue;
		}
		uint32_t const y = perm[i];
		uint32_t const x = std::distance(
		    perm.begin(), std::find(perm.begin() + i, perm.end(), i));

		if (__builtin_popcount(i ^ y) <= __builtin_popcount(i ^ x)) {
			uint32_t const p = i & ~y;
			if (p) {
				update_permutation(perm, y, p);
				pos = gates.emplace(pos, y, p);
			}
			uint32_t const q = ~i & y;
			if (q) {
				update_permutation(perm, i, q);
				pos = gates.emplace(pos, i, q);
			}
			continue;
		}
		uint32_t const p = ~x & i;
		if (p) {
			update_permutation_inv(perm, x, p);
			pos = gates.emplace(pos, x, p);
			++pos;
		}
		uint32_t const q = x & ~i;
		if (q) {
			update_permutation_inv(perm, i, q);
			pos = gates.emplace(pos, i, q);
			++pos;
		}
	}
	return gates;
}

inline GateList multidirectional(std::vector<uint32_t> perm)
{
	GateList gates;
	auto pos = gates.begin();
	for (uint32_t i = 0u; i < perm.size(); ++i) {
		// Find cheapest assignment
		uint32_t x_best = i;
		uint32_t x_best_cost = __builtin_popcount(i ^ perm[i]);
		for (uint32_t j = i + 1; j < perm.size(); ++j) {
			uint32_t j_cost = __builtin_popcount(i ^ perm[j]);
			uint32_t cost = __builtin_popcount(i ^ j) + j_cost;
			if (cost < x_best_cost) {
				x_best = j;
				x_best_cost = cost;
			}
		}

		uint32_t const y = perm[x_best];
		// map x |-> i
		uint32_t p = ~x_best & i;
		if (p) {
			update_permutation_inv(perm, x_best, p);
			pos = gates.emplace(pos, x_best, p);
			pos++;
		}
		uint32_t q = x_best & ~i;
		if (q) {
			update_permutation_inv(perm, i, q);
			pos = gates.emplace(pos, i, q);
			pos++;
		}

		// map y |-> i
		p = i & ~y;
		if (p) {
			update_permutation(perm, y, p);
			pos = gates.emplace(pos, y, p);
		}
		q = ~i & y;
		if (q) {
			update_permutation(perm, i, q);
			pos = gates.emplace(pos, i, q);
		}
	}
	return gates;
}

inline void synthesize(Circuit& circuit, std::vector<WireRef> const& qubits,
    std::vector<uint32_t> const& perm)
{
	// GateList gates = unidirectional(perm);
	// GateList gates = bidirectional(perm);
	GateList gates = multidirectional(perm);
	for (auto [controls, targets] : gates) {
		std::vector<WireRef> cs;
		for (uint32_t c = 0; controls; controls >>= 1, ++c) {
			if (controls & 1) {
				cs.emplace_back(qubits.at(c));
			}
		}
		for (uint32_t t = 0; targets; targets >>= 1, ++t) {
			if ((targets & 1) == 0) {
				continue;
			}
			circuit.create_instruction(
			    GateLib::X(), {cs}, qubits.at(t));
		}
	}
}

} // namespace transform_synth_detail
#pragma endregion

/*! \brief Reversible synthesis based on functional decomposition.
 *
 * This is the in-place variant of ``transform_synth`` in which the circuit is
 * passed as a parameter and can potentially already contain some gates.  The
 * parameter ``qubits`` provides a qubit mapping to the existing qubits in the
 * circuit.
 * 
 * \param[inout] circuit A circuit in which the permutation will be synthesized
 * on.
 * \param[in] qubits The wires that will be used.
 * \param[in] perm A vector of different integers.
 */
inline void transform_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    std::vector<uint32_t> const& perm)
{
	assert(!perm.empty() && !(perm.size() & (perm.size() - 1)));
	transform_synth_detail::synthesize(circuit, qubits, perm);
}

/*! \brief Reversible synthesis based on functional decomposition.
 *
 * A permutation is specified as a vector of :math:`2^n` different integers
 * ranging from :math:`0` to :math:`2^n-1`.
 * 
 * \param[in] perm A vector of different integers.
 * \return A reversible circuit.
 */
inline Circuit transform_synth(std::vector<uint32_t> const& perm)
{
	assert(!perm.empty() && !(perm.size() & (perm.size() - 1)));
	// TODO: method to generate a name;
	Circuit circuit("my_circuit");

	// Create the necessary qubits
	uint32_t const num_qubits = __builtin_ctz(perm.size());
	std::vector<WireRef> wires;
	wires.reserve(num_qubits);
	for (uint32_t i = 0u; i < num_qubits; ++i) {
		wires.emplace_back(circuit.create_qubit());
	}
	transform_synth(circuit, wires, perm);
	return circuit;
}

} // namespace tweedledum
