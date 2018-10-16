/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_kinds.hpp"

#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <kitty/detail/mscfix.hpp>
#include <string>

namespace tweedledum {

inline auto make_qubit_list(std::string& s)
{
	return [&](auto c) {
		if (!s.empty()) {
			s += ", ";
		}
		s += fmt::format("qs[{}]", c);
	};
}

/*! \brief Writes network in ProjecQ format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required gate functions:**
 * - `kind`
 * - `num_controls`
 * - `foreach_control`
 * - `foreach_target`
 * 
 * **Required network functions:**
 * - `num_qubits`
 * - `foreach_node`
 *
 * \param network Network
 * \param os Output stream
 */
template<typename Network>
void write_projectq(Network const& network, std::ostream& os)
{
	network.foreach_node([&](auto const& n) {
		auto const& g = n.gate;

		std::string controls, targets;

		g.foreach_control(make_qubit_list(controls));
		g.foreach_target(make_qubit_list(targets));

		switch (g.kind()) {
		default:
			std::cout << "[w] unknown gate kind " << static_cast<uint32_t>(g.kind())
			          << "\n";
			assert(false);
			break;
		case gate_kinds_t::input:
		case gate_kinds_t::output:
			/* ignore */
			break;
		case gate_kinds_t::hadamard:
			os << fmt::format("H | {}\n", targets);
			break;
		case gate_kinds_t::pauli_x:
			os << fmt::format("X | {}\n", targets);
			break;
		case gate_kinds_t::pauli_y:
			os << fmt::format("Y | {}\n", targets);
			break;
		case gate_kinds_t::pauli_z:
			os << fmt::format("Z | {}\n", targets);
			break;
		case gate_kinds_t::phase:
			os << fmt::format("S | {}\n", targets);
			break;
		case gate_kinds_t::phase_dagger:
			os << fmt::format("Sdag | {}\n", targets);
			break;
		case gate_kinds_t::t:
			os << fmt::format("T | {}\n", targets);
			break;
		case gate_kinds_t::t_dagger:
			os << fmt::format("Tdag | {}\n", targets);
			break;
		case gate_kinds_t::rotation_x:
			os << fmt::format("Rx({}) | {}\n", g.rotation_angle(), targets);
			break;
		case gate_kinds_t::rotation_z:
			os << fmt::format("Rz({}) | {}\n", g.rotation_angle(), targets);
			break;
		case gate_kinds_t::cx:
			os << fmt::format("CNOT | ({}, {})\n", controls, targets);
			break;
		case gate_kinds_t::cz:
			os << fmt::format("CZ | ({}, {})\n", controls, targets);
			break;
		case gate_kinds_t::mcx:
			os << fmt::format("C(All(X), {}) | ([{}], [{}])\n", g.num_controls(),
			                   controls, targets);
			break;
		case gate_kinds_t::mcy:
			os << fmt::format("C(All(Y), {}) | ([{}], [{}])\n", g.num_controls(),
			                   controls, targets);
			break;
		case gate_kinds_t::mcz:
			os << fmt::format("C(All(Z), {}) | ([{}], [{}])\n", g.num_controls(),
			                   controls, targets);
			break;
		}
	});
}

/*! \brief Writes network in ProjecQ format into a file
 *
 * **Required gate functions:**
 * - `kind`
 * - `num_controls`
 * - `foreach_control`
 * - `foreach_target`
 * 
 * **Required network functions:**
 * - `num_qubits`
 * - `foreach_node`
 *
 * \param network Network
 * \param filename Filename
 */
template<typename Network>
void write_projectq(Network const& network, std::string const& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_projectq(network, os);
}

} // namespace tweedledum
