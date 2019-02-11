/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_base.hpp"
#include "../../networks/qubit.hpp"
#include "../../utils/zdd.hpp"
#include "../../utils/device.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace tweedledum {

#pragma region utility functions(for readability)
namespace utility {
template<class Circuit>
void add_qubits(Circuit& circ, uint32_t num_qubits)
{
	for (auto i = 0u; i < num_qubits; ++i) {
		circ.add_qubit();
	}
}

template<class Circuit>
void add_swap(Circuit& circ, uint32_t a, uint32_t b)
{
	circ.add_gate(gate::cx, qubit_id(a), qubit_id(b));
	circ.add_gate(gate::cx, qubit_id(b), qubit_id(a));
	circ.add_gate(gate::cx, qubit_id(a), qubit_id(b));
}

template<class Gate>
qubit_id get_control(Gate const& gate)
{
	qubit_id control;
	gate.foreach_control([&](auto qid) {
		control = qid;
		return false;
	});
	return control;
}

template<class Gate>
qubit_id get_target(Gate const& gate)
{
	qubit_id target;
	gate.foreach_target([&](auto qid) {
		target = qid;
		return false;
	});
	return target;
}

zdd_base swap_circuits(device const& arch)
{
	zdd_base zdd_map(arch.edges.size());
	zdd_map.build_tautologies();
	return zdd_map;
}

} // namespace utility
#pragma endregion

/*! \brief A very simple Greedy mapper.
 *
 * This mapper should only be used as a baseline for new implementations or evaluations.  It is one
 * of the most simple possible mappers, yet using ZDDs to sample SWAP circuits.
 *
 * The initial mapping is the identity permutation.  It then adds gates from `circ` to the resulting
 * circuit as long as the 2-input gates match the coupling constraints from `arch`.  At conflict,
 * the mapper tries all possible SWAP circuits and takes the first one which allows the algorithm to
 * map the next gate.
 *
 * There are two cases in which no mapping can be performed for `circ`. (i) the circuit uses more
 * qubits than available in `arch`, and (ii) the circuit has gates that act on more than 2 controls.
 * In these two cases, the circuit returns `std::nullopt`, otherwise an optional containing the
 * mapped circuit.
 */
template<class Circuit>
std::optional<Circuit> greedy_map(Circuit const& circ, device const& arch)
{
	using namespace utility;

	if (circ.num_qubits() > arch.num_vertices) {
		return std::nullopt;
	}

	Circuit res_circ;
	add_qubits(res_circ, arch.num_vertices);

	auto d = arch.get_coupling_matrix();
	auto swaps = swap_circuits(arch);

	bool error = false;
	circ.foreach_cgate([&](auto const& n) {
		if (n.gate.is_single_qubit()) {
			const auto target = get_target(n.gate);
			res_circ.add_gate(n.gate, qubit_id(target));
		} else if (n.gate.is_double_qubit()) {
			const auto c = get_control(n.gate);
			const auto t = get_target(n.gate);

			if (d.at(c, t)) { 
				/* gate can be mapped */
				res_circ.add_gate(n.gate, qubit_id(c), qubit_id(t));
				return true;
			}
			/* gate cannot be mapped */
			/* try SWAPs to add next gate */
			swaps.foreach_set(swaps.tautology(), [&](auto const& swap_set) {
				auto new_mapping = res_circ.rewire_map();
				for (auto e : swap_set) {
					std::swap(new_mapping[arch.edges[e].first],
						  new_mapping[arch.edges[e].second]);
				}
				auto mc = new_mapping[c];
				auto mt = new_mapping[t];
				if (d.at(mc, mt)) {
					res_circ.rewire(new_mapping);
					for (auto e : swap_set) {
						add_swap(res_circ, arch.edges[e].first,
							 arch.edges[e].second);
					}
					res_circ.add_gate(n.gate, qubit_id(c), qubit_id(t));
					return false; /* break */
				}
				return true; /* continue */
			});
		} else {
			error = true;
			return false;
		}
		return true;
	});
	if (error) {
		return std::nullopt;
	}
	return res_circ;
}

} // namespace tweedledum
