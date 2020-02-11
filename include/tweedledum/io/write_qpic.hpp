/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_lib.hpp"
#include "../utils/angle.hpp"

#include <fmt/format.h>
#include <fstream>
#include <iostream>

namespace tweedledum {

/*! \brief Writes network in qpic format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * \param network A quantum network
 * \param os Output stream
 * \param color_marked_gates Flag to draw marked nodes in red
 */
template<typename Network>
void write_qpic(Network const& network, std::ostream& os, bool color_marked_gates = false)
{
	if (color_marked_gates) {
		os << "DEFINE mark color=red:style=thick\n";
	}
	network.foreach_io([&](io_id id, auto const& name) {
		if (id.is_qubit()) {
			os << fmt::format("id{} W {} {}\n", id, name, name);
		} else {
			os << fmt::format("id{} W {} {} cwire\n", id, name, name);
		}
	});

	network.foreach_gate([&](auto const& node) {
		auto prefix = "";
		if (node.gate.is(gate_lib::mcx)) {
			prefix = "+";
		}
		node.gate.foreach_target([&](auto qubit) {
			os << fmt::format("{}id{} ", prefix, qubit);
		});
		switch (node.gate.operation()) {
		case gate_lib::cx:
			os << 'C';
			break;

		case gate_lib::mcx:
			break;

		case gate_lib::cz:
		case gate_lib::mcz:
			os << 'Z';
			break;

		case gate_lib::hadamard:
			os << 'H';
			break;

		case gate_lib::rx: {
			angle rotation_angle = node.gate.rotation_angle();
			if (rotation_angle == angles::pi) {
				os << 'N';
			} else {
				os << "G $R_{x}$";
			}
		} break;

		case gate_lib::rz: {
			angle rotation_angle = node.gate.rotation_angle();
			if (rotation_angle == angles::pi_quarter) {
				os << "G $T$";
			} else if (rotation_angle == -angles::pi_quarter) {
				os << "G $T^{\\dagger}$";
			} else if (rotation_angle == angles::pi_half) {
				os << "G $S$";
			} else if (rotation_angle == -angles::pi_half) {
				os << "G $S^{\\dagger}$";
			} else if (rotation_angle == angles::pi) {
				os << 'Z';
			} else {
				os << "G $R_{z}$";
			}
		} break;

		case gate_lib::swap:
			os << "SWAP";
			break;

		default:
			break;
		}
		node.gate.foreach_control([&](auto qubit) {
			os << fmt::format(" {}id{}", qubit.is_complemented() ? "-" : "", qubit); 
		});
		os << '\n';
	});
}

/*! \brief Writes network in qpic format into a file
 *
 * \param network A quantum network
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
