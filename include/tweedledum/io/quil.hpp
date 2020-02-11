/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_lib.hpp"
#include "../networks/io_id.hpp"
#include "../utils/angle.hpp"

#include <cassert>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <tweedledee/quil/ast/visitor.hpp>
#include <tweedledee/quil/quil.hpp>

namespace tweedledum {

/*! \brief Reads OPENQASM 2.0 format
 */
template<typename Network>
Network read_quil_from_file(std::string const& path)
{
	using namespace tweedledee::quil;
	Network network;
	auto program_ast = read_from_file(path);
	if (program_ast) {
		ast_printer printer;
		printer.visit(*program_ast);
	}
	return network;
}

/*! \brief Writes network in quil format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * \param network A quantum network
 * \param os Output stream
 */
template<typename Network>
void write_quil(Network const& network, std::ostream& os)
{
	network.foreach_gate([&](auto const& node) {
		auto const& gate = node.gate;
		switch (gate.operation()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			assert(0);
			return true;

		case gate_lib::hadamard:
			gate.foreach_target([&](auto target) { os << fmt::format("H {}\n", target); });
			break;

		case gate_lib::rx: {
			angle rotation_angle = gate.rotation_angle();
			if (rotation_angle == angles::pi) {
				gate.foreach_target([&](auto target) { 
					os << fmt::format("X {}\n", target); 
				});
			} else if (rotation_angle == angles::pi_half) {
				gate.foreach_target([&](auto target) { 
					os << fmt::format("RX(pi/2) {}\n", target); 
				});
			} else if (rotation_angle == -angles::pi_half) {
				gate.foreach_target([&](auto target) { 
					os << fmt::format("RX(-pi/2) {}\n", target); 
				});
			} else {
				std::cerr << "[w] unsupported gate type\n";
				assert(0);
			}
		} break;

		case gate_lib::rz: {
			angle rotation_angle = gate.rotation_angle();
			std::string symbol;
			if (rotation_angle == angles::pi_quarter) {
				symbol = "T";
			} else if (rotation_angle == angles::pi_half) {
				symbol = "S";
			} else if (rotation_angle == angles::pi) {
				symbol = "Z";
			} else {
				gate.foreach_target([&](auto target) {
					os << fmt::format("RZ({}) {}\n", rotation_angle, target);
				});
				break;
			}
			gate.foreach_target([&](auto target) {
				os << fmt::format("{} {}\n", symbol, target);
			});
		} break;

		case gate_lib::cx:
			gate.foreach_control([&](auto control) {
				if (control.is_complemented()) {
					os << fmt::format("X {}\n", control.index());
				}
				gate.foreach_target([&](auto target) {
					os << fmt::format("CNOT {} {}\n", control.index(), target); 
				});
				if (control.is_complemented()) {
					os << fmt::format("X {}\n", control.index());
				}
			});
			break;
                
                case gate_lib::cz:
			gate.foreach_control([&](auto control) {
				if (control.is_complemented()) {
					os << fmt::format("X {}\n", control.index());
				}
				gate.foreach_target([&](auto target) {
					os << fmt::format("CZ {} {}\n", control.index(), target); 
				});
				if (control.is_complemented()) {
					os << fmt::format("X {}\n", control.index());
				}
			});
                        break;
		
		case gate_lib::swap: {
			std::vector<io_id> targets;
			gate.foreach_target([&](auto target) {
				targets.push_back(target);
			});
			os << fmt::format("CNOT {} {}\n", targets[0], targets[1]);
			os << fmt::format("CNOT {} {}\n", targets[1], targets[0]);
			os << fmt::format("CNOT {} {}\n", targets[0], targets[1]);
		} break;

		case gate_lib::mcx: {
			std::vector<io_id> controls;
			std::vector<io_id> targets;
			gate.foreach_control([&](auto control) {
				if (control.is_complemented()) {
					os << fmt::format("X {}\n", control.index());
				}
				controls.emplace_back(control.id()); 
			});
			gate.foreach_target([&](auto target) {
				targets.push_back(target);
			});
			switch (controls.size()) {
			default:
				std::cerr << "[w] unsupported control size\n";
				assert(0);
				return true;

			case 0u:
				for (auto target : targets) {
					os << fmt::format("X {}\n", target);
				}
				break;

			case 1u:
				for (auto target : targets) {
					os << fmt::format("CNOT {} {}\n", controls[0], target);
				}
				break;

			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("CNOT {} {}\n", targets[0], targets[i]);
				}
				os << fmt::format("CCNOT {} {} {}\n", controls[0], controls[1],
				                   targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("CNOT {} {}\n", targets[0], targets[i]);
				}
				break;
			}
			gate.foreach_control([&](auto control) {
				if (control.is_complemented()) {
					os << fmt::format("X {}\n", control.index());
				}
			});
		} break;
		}
		return true;
	});
}

/*! \brief Writes network in quil format into a file
 *
 * \param network A quantum network
 * \param filename Filename
 */
template<typename Network>
void write_quil(Network const& network, std::string const& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_quil(network, os);
}

} // namespace tweedledum
