/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

#include <fmt/format.h>
#include <fstream>
#include <iostream>

namespace tweedledum {

/*! \brief Writes network in qpic format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required gate functions:**
 * - `kind`
 * - `is`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `num_qubits`
 * - `foreach_node`
 * - `foreach_qubit`
 *
 * \param network Network
 * \param os Output stream
 * \param color_marked_gates Flag to draw marked nodes in red
 */
template<typename Network>
void write_qpic(Network const& network, std::ostream& os, bool color_marked_gates = false)
{
	if (color_marked_gates) {
		os << "DEFINE mark color=red:style=thick\n";
	}
	network.foreach_qubit([&](auto id, auto const& name) {
		os << fmt::format("q{} W {} {}\n", id, name, name);
	});

	network.foreach_gate([&](auto& node) {
		auto prefix = "";
		if (node.gate.is(gate_kinds_t::mcx)) {
			prefix = "+";
		}
		node.gate.foreach_target(
		    [&](auto qubit) { os << fmt::format("{}q{} ", prefix, qubit); });
		switch (node.gate.kind()) {
		case gate_kinds_t::pauli_x:
			os << 'N';
			break;

		case gate_kinds_t::cx:
			os << 'C';
			break;

		case gate_kinds_t::mcx:
			break;

		case gate_kinds_t::pauli_z:
		case gate_kinds_t::cz:
		case gate_kinds_t::mcz:
			os << 'Z';
			break;

		case gate_kinds_t::hadamard:
			os << 'H';
			break;

		case gate_kinds_t::phase:
			os << "G $P$";
			break;

		case gate_kinds_t::phase_dagger:
			os << "G $P^{\\dagger}$";
			break;

		case gate_kinds_t::t:
			os << "G $T$";
			break;

		case gate_kinds_t::t_dagger:
			os << "G $T^{\\dagger}$";
			break;

		case gate_kinds_t::rotation_x:
			os << "G $R_{x}$";
			break;

		case gate_kinds_t::rotation_z:
			os << "G $R_{z}$";
			break;

		default:
			break;
		}
		node.gate.foreach_control([&](auto qubit) { os << fmt::format(" q{}", qubit); });
		os << fmt::format("{}", color_marked_gates && network.mark(node) ? " mark\n" : "\n");
	});
}

/*! \brief Writes network in qpic format into a file
 *
 * **Required gate functions:**
 * - `kind`
 * - `is`
 * - `foreach_control`
 * - `foreach_target`
 *
 * **Required network functions:**
 * - `num_qubits`
 * - `foreach_node`
 * - `foreach_qubit`
 *
 * \param network Network
 * \param filename Filename
 * \param color_marked_gates Flag to draw marked nodes in red
 */
template<typename Network>
void write_qpic(Network const& network, std::string const& filename, bool color_marked_gates = false)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_qpic(network, os, color_marked_gates);
}

} // namespace tweedledum
