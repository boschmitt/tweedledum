/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../ir/Circuit.h"
#include "../../ir/GateLib.h"
#include "../../ir/Wire.h"

#include <list>

// This implementation is based on:
//
// De Vos, Alexis, and Yvan Van Rentergem. "Young subgroups for reversible
// computers." Advances in Mathematics of Communications 2.2 (2008): 183.
//
// In decomposition-based synthesis the reversible function is recursively
// decomposed into simpler functions based on the Young subgroup decomposition:
//
//     Given a wire Wi, every reversible function f can be decomposed into three 
//     functions f = g1 o f' o g2, where g1 and g2 can be realized with a
//     truth table gate on Wi and f' is a reversible function that does not
//     change in Wi.
//
// Based on this decomposition, this synthesis algorithms determine the gates 
// for g1 and g2 and then recur on f'.
//
namespace tweedledum {
#pragma region Implementation details
namespace decomp_synth_detail {

inline auto decompose(std::vector<uint32_t>& perm, uint32_t var)
{
	std::vector<uint32_t> left(perm.size(), 0);
	std::vector<uint32_t> right(perm.size(), 0);
	std::vector<uint8_t> visited(perm.size(), 0);

	uint32_t row = 0u;
	do {
		// Assign 0 to var on left side
		left[row] = (row & ~(1 << var));
		visited[row] = 1;
		// Assign 1 to var on left side
		left[row ^ (1 << var)] = left[row] ^ (1 << var);
		row ^= (1 << var);
		visited[row] = 1;

		// Assign 1 to var on right side
		right[perm[row] | (1 << var)] = perm[row];
		// Assign 0 to var on right side
		right[perm[row] & ~(1 << var)] = perm[row] ^ (1 << var);

		// Find next row
		uint32_t i = 0;
		for (; i < perm.size() - 1; ++i) {
			if (perm[i] == (perm[row] ^ (1 << var))) {
				break;
			}
		}
		// If this rows was already visited, find a new one.
		if (visited[i]) {
			for (i = 0; i < perm.size(); ++i) {
				if (visited[i] == 0) {
					break;
				}
			}
			// If there is no unvisited rows, we are done!
			if (i == perm.size()) {
				break;
			}
		}
		row = i;
	} while (true);

	std::vector<uint32_t> perm_old = perm;
	for (uint32_t i = 0; i < perm.size(); ++i) {
		perm[left[i]] = right[perm_old[i]];
	}
	return std::make_pair(std::move(left), std::move(right));
}

inline void synthesize(Circuit& circuit, std::vector<WireRef> const& qubits,
    std::vector<uint32_t> perm)
{
	std::vector<kitty::dynamic_truth_table> truth_tables;
	truth_tables.reserve(qubits.size() * 2);
	std::list<std::pair<uint32_t, std::vector<WireRef>>> gates;
	auto pos = gates.begin();
	for (uint32_t i = 0u; i < qubits.size(); ++i) {
		auto&& [left, right] = decompose(perm, i);

		auto& left_tt = truth_tables.emplace_back(qubits.size());
		auto& right_tt = truth_tables.emplace_back(qubits.size());
		for (uint32_t row = 0; row < perm.size(); ++row) {
			if (left[row] != row) {
				kitty::set_bit(left_tt, row);
			}
			if (right[row] != row) {
				kitty::set_bit(right_tt, row);
			}
		}

		if (!kitty::is_const0(left_tt)) {
			std::vector<WireRef> left_qubits;
			for (auto element : kitty::min_base_inplace(left_tt)) {
				left_qubits.emplace_back(qubits.at(element));
			}
			left_tt = kitty::shrink_to(left_tt, left_qubits.size());
			left_qubits.push_back(qubits.at(i));
			pos = gates.emplace(pos, truth_tables.size() - 2,
			    std::move(left_qubits));
			++pos;
		}
		if (!kitty::is_const0(right_tt)) {
			std::vector<WireRef> right_qubits;
			for (auto element : kitty::min_base_inplace(right_tt)) {
				right_qubits.emplace_back(qubits.at(element));
			}
			right_tt = kitty::shrink_to(right_tt, right_qubits.size());
			right_qubits.push_back(qubits.at(i));
			pos = gates.emplace(pos, truth_tables.size() - 1,
			    std::move(right_qubits));
		}
	}
	for (auto& [tt_idx, qubits] : gates) {
		circuit.create_instruction(
		    GateLib::TruthTable("f", truth_tables.at(tt_idx)), qubits);
	}
}

} // namespace decomp_synth_detail
#pragma endregion

/*! \brief Reversible synthesis based on functional decomposition.
 *
 * This is the in-place variant of ``decomp_synth`` in which the circuit is
 * passed as a parameter and can potentially already contain some gates.  The
 * parameter ``qubits`` provides a qubit mapping to the existing qubits in the
 * circuit.
 * 
 * \param[inout] circuit A circuit in which the permutation will be synthesized
 * on.
 * \param[in] qubits The wires that will be used.
 * \param[in] perm A vector of different integers.
 */
inline void decomp_synth(Circuit& circuit, std::vector<WireRef> const& qubits,
    std::vector<uint32_t> const& perm)
{
	assert(!perm.empty() && !(perm.size() & (perm.size() - 1)));
	decomp_synth_detail::synthesize(circuit, qubits, perm);
}

/*! \brief Reversible synthesis based on functional decomposition.
 *
 * A permutation is specified as a vector of :math:`2^n` different integers
 * ranging from :math:`0` to :math:`2^n-1`.
 * 
 * \param[in] perm A vector of different integers.
 * \return A reversible circuit.
 */
inline Circuit decomp_synth(std::vector<uint32_t> const& perm)
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
	decomp_synth(circuit, wires, perm);
	return circuit;
}

} // namespace tweedledum
