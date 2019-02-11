/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../gates/gate_base.hpp"
#include "../../networks/qubit.hpp"
#include "device.hpp"
#include "zdd.hpp"

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
uint32_t get_control(Gate const& g)
{
	uint32_t c{0};
	g.foreach_control([&](auto _c) {
		c = _c;
		return false;
	});
	return c;
}

template<class Gate>
uint32_t get_target(Gate const& g)
{
	uint32_t t{0};
	g.foreach_target([&](auto _t) {
		t = _t;
		return false;
	});
	return t;
}

zdd_base swap_circuits(device_t const& arch)
{
	zdd_base zdd_map(arch.edges.size());
	zdd_map.build_tautologies();
	return zdd_map;
}

} // namespace utility
#pragma endregion

/*! \brief A very simple Greedy mapper.
 *
 * This mapper should only be used as a baseline for new implementations or
 * evaluations.  It is one of the most simple possible mappers, yet using ZDDs
 * to sample SWAP circuits.
 *
 * The initial mapping is the identity permutation.  It then adds gates from
 * `circ` to the resulting circuit as long as the 2-input gates match the
 * coupling constraints from `arch`.  At conflict, the mapper tries all possible
 * SWAP circuits and takes the first one which allows the algorithm to map the
 * next gate.
 *
 * There are two cases in which no mapping can be performed for `circ`. (i) the
 * circuit uses more qubits than available in `arch`, and (ii) the circuit has
 * gates that act on more than 2 controls.  In these two cases, the circuit
 * returns `std::nullopt`, otherwise an optional containing the mapped circuit.
 */
template<class Circuit>
std::optional<Circuit> greedy_map(Circuit& circ, device_t const& arch)
{
	using namespace utility;

	if (circ.num_qubits() > arch.num_vertices) {
		return std::nullopt;
	}

	Circuit res_circ;
	add_qubits(res_circ, arch.num_vertices);

	/* initial mapping */
	std::vector<uint32_t> mapping(arch.num_vertices);
	std::iota(mapping.begin(), mapping.end(), 0u);

	auto d = arch.get_coupling_matrix();
	auto swaps = swap_circuits(arch);

	bool error{false};
	circ.foreach_cgate([&](auto const& n) {
		if (n.gate.is_single_qubit()) {
			auto mt = mapping[get_target(n.gate)];
			res_circ.add_gate(n.gate, qubit_id(mt));
		} else if (n.gate.is_double_qubit()) {
			const auto c = get_control(n.gate);
			const auto t = get_target(n.gate);
			auto mc = mapping[c];
			auto mt = mapping[t];

			if (d.at(mc, mt)) { /* gate can be mapped */
				res_circ.add_gate(n.gate, qubit_id(mc), qubit_id(mt));
			} else { /* gate cannot be mapped */
				/* try SWAPs to add next gate */
				swaps.foreach_set(swaps.tautology(), [&](auto const& swap_set) {
					auto new_mapping = mapping;
					for (auto e : swap_set) {
						std::swap(new_mapping[arch.edges[e].first],
						          new_mapping[arch.edges[e].second]);
					}
					mc = new_mapping[c];
					mt = new_mapping[t];
					if (d.at(mc, mt)) {
						mapping = new_mapping;
						for (auto e : swap_set) {
							add_swap(res_circ, arch.edges[e].first,
							         arch.edges[e].second);
						}
						res_circ.add_gate(n.gate, qubit_id(mc), qubit_id(mt));
						return false; /* break */
					}
					return true; /* continue */
				});
			}
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
