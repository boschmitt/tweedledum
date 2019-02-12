/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_set.hpp"

#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <kitty/detail/mscfix.hpp>
#include <string>

namespace tweedledum {

/*! \brief Writes network in ProjecQ format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required gate functions:**
 * - `foreach_control`
 * - `foreach_target`
 * - `op`
 *
 * **Required network functions:**
 * - `foreach_cgate`
 * - `foreach_cqubit`
 *
 * \param network Network
 * \param os Output stream
 */
template<typename Network>
void write_projectq(Network const& network, std::ostream& os = std::cout)
{
	network.foreach_cgate([&](auto const& node) {
		auto const& gate = node.gate;

		std::string controls;
		std::string negative_controls;
		gate.foreach_control([&](auto control) {
			if (!controls.empty()) {
				controls += ", ";
			}
			controls += fmt::format("qs[{}]", control.index());
			if (control.is_complemented()) {
				if (!negative_controls.empty()) {
					negative_controls += ", ";
				}
				negative_controls += fmt::format("qs[{}]", control.index());
			}
		});

		std::string targets;
		gate.foreach_target([&](auto target) {
			if (!targets.empty()) {
				targets += ", ";
			}
			targets += fmt::format("qs[{}]", target.index());
		});

		if (!negative_controls.empty()) {
			os << fmt::format("X | {}\n", negative_controls);
		}
		switch (gate.operation()) {
		default:
			std::cout << "[w] unknown gate kind \n";
			assert(false);
			break;

		case gate_set::hadamard:
			os << fmt::format("H | {}\n", targets);
			break;

		case gate_set::pauli_x:
			os << fmt::format("X | {}\n", targets);
			break;

		case gate_set::pauli_y:
			os << fmt::format("Y | {}\n", targets);
			break;

		case gate_set::pauli_z:
			os << fmt::format("Z | {}\n", targets);
			break;

		case gate_set::phase:
			os << fmt::format("S | {}\n", targets);
			break;

		case gate_set::phase_dagger:
			os << fmt::format("Sdag | {}\n", targets);
			break;

		case gate_set::t:
			os << fmt::format("T | {}\n", targets);
			break;

		case gate_set::t_dagger:
			os << fmt::format("Tdag | {}\n", targets);
			break;

		case gate_set::rotation_x:
			os << fmt::format("Rx({}) | {}\n", gate.rotation_angle().numeric_value(),
			                  targets);
			break;

		case gate_set::rotation_z:
			os << fmt::format("Rz({}) | {}\n", gate.rotation_angle().numeric_value(),
			                  targets);
			break;

		case gate_set::cx:
			os << fmt::format("CNOT | ({}, {})\n", controls, targets);
			break;

		case gate_set::cz:
			os << fmt::format("CZ | ({}, {})\n", controls, targets);
			break;

		case gate_set::mcx:
			os << fmt::format("C(All(X), {}) | ([{}], [{}])\n", gate.num_controls(),
			                  controls, targets);
			break;

		case gate_set::mcz:
			os << fmt::format("C(All(Z), {}) | ([{}], [{}])\n", gate.num_controls(),
			                  controls, targets);
			break;
		}
		if (!negative_controls.empty()) {
			os << fmt::format("X | {}\n", negative_controls);
		}
	});
}

/*! \brief Writes network in ProjecQ format into a file
 *
 * **Required gate functions:**
 * - `foreach_control`
 * - `foreach_target`
 * - `op`
 *
 * **Required network functions:**
 * - `foreach_cgate`
 * - `foreach_cqubit`
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
