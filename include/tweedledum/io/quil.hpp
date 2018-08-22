/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
|            Bruno Schmitt
*-----------------------------------------------------------------------------*/
#pragma once

#include "../networks/gates/gate_kinds.hpp"
#include "../networks/netlist.hpp"

#include <cstdint>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>
#include <tweedledee/quil/ast/visitor.hpp>
#include <tweedledee/quil/quil.hpp>

namespace tweedledum {

template<typename Network>
void read_quil_file(Network& circ, std::string const& path)
{
	using namespace tweedledee::quil;

	auto program = quil_read_file(path);
	for (auto& qubit_label : program->qubits) {
		circ.add_qubit(qubit_label);
	}
	for (auto& child : *program) {
		auto& g = static_cast<const stmt_gate&>(child);
		if (g.identifier == "RX") {
			auto angle
			    = static_cast<const expr_real&>(*g.begin()).evaluate();
			auto label = static_cast<const qubit&>(*g.back()).label;
			circ.add_x_rotation(label, angle);
		} else if (g.identifier == "RZ") {
			auto angle
			    = static_cast<const expr_real&>(*g.begin()).evaluate();
			auto label = static_cast<const qubit&>(*g.back()).label;
			circ.add_z_rotation(label, angle);
		} else if (g.identifier == "CZ") {
			auto label0 = static_cast<const qubit&>(*g.begin()).label;
			auto label1 = static_cast<const qubit&>(*g.back()).label;
			circ.add_controlled_gate(gate_kinds_t::cz, label0,
			                         label1);
		} else if (g.identifier == "CX") {
			auto label0 = static_cast<const qubit&>(*g.begin()).label;
			auto label1 = static_cast<const qubit&>(*g.back()).label;
			circ.add_controlled_gate(gate_kinds_t::cx, label0,
			                         label1);
		} else if (g.identifier == "SWAP") {
			auto label0 = static_cast<const qubit&>(*g.begin()).label;
			auto label1 = static_cast<const qubit&>(*g.back()).label;
			circ.add_controlled_gate(gate_kinds_t::cx, label0,
			                         label1);
			circ.add_controlled_gate(gate_kinds_t::cx, label1,
			                         label0);
			circ.add_controlled_gate(gate_kinds_t::cx, label0,
			                         label1);
		} else {
			std::cout << "[w] cannot process gate " << g.identifier << "\n";
		}
	}
	// visit(*program, [&](const ast_node& node, visitor_info info) {
	// 	switch (node.kind()) {
	// 		case ast_node_kinds::program:
	// 			return true:
	// 		default:
	// 			return true;
	// 	}
	// 	if (node.kind() == ast_node_kinds::program) {
	// 		return true;
	// 	}
	// 	else if (info == visitor_info::container_end) {
	// 		prefix.pop_back();
	// 		prefix.pop_back();
	// 	} else {
	// 		std::cout << prefix;
	// 		if (info == visitor_info::container_begin)
	// 			prefix += "| ";
	// 		std::cout << "|-";
	// 		node.print(std::cout);
	// 		std::cout << '\n';
	// 	}
	// 	return true;
	// });
}

template<typename Network>
void write_quil(Network const& circ, std::ostream& out)
{
	circ.foreach_node([&](auto const& n) {
		auto const& g = n.gate;
		switch (g.kind()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			return true;

		case gate_kinds_t::input:
		case gate_kinds_t::output:
			/* ignore */
			return true;

		case gate_kinds_t::hadamard: {
			g.foreach_target(
			    [&](auto q) { out << fmt::format("H {}\n", q); });
		} break;

		case gate_kinds_t::pauli_x: {
			g.foreach_target(
			    [&](auto q) { out << fmt::format("X {}\n", q); });
		} break;

		case gate_kinds_t::t: {
			g.foreach_target(
			    [&](auto q) { out << fmt::format("T {}\n", q); });
		} break;

		case gate_kinds_t::t_dagger: {
			g.foreach_target([&](auto q) {
				out << fmt::format("RZ(-pi/4) {}\n", q);
			});
		} break;

		case gate_kinds_t::rotation_z: {
			g.foreach_target([&](auto qt) {
				out << fmt::format("RZ({}) {}\n", g.angle(), qt);
			});
		} break;

		case gate_kinds_t::cx: {
			g.foreach_control([&](auto qc) {
				g.foreach_target([&](auto qt) {
					out << fmt::format("CNOT {} {}\n", qc,
					                   qt);
				});
			});
		} break;

		case gate_kinds_t::mcx: {
			std::vector<uint32_t> controls, targets;
			g.foreach_control([&](auto q) { controls.push_back(q); });
			g.foreach_target([&](auto q) { targets.push_back(q); });
			switch (controls.size()) {
			default:
				std::cerr << "[w] unsupported control size\n";
				return true;
			case 0u:
				for (auto q : targets) {
					out << fmt::format("X {}\n", q);
				}
				break;
			case 1u:
				for (auto q : targets) {
					out << fmt::format("CNOT {} {}\n",
					                   controls[0], q);
				}
				break;
			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					out << fmt::format("CNOT {} {}\n",
					                   targets[0],
					                   targets[i]);
				}
				out << fmt::format("CCNOT {} {} {}\n",
				                   controls[0], controls[1],
				                   targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					out << fmt::format("CNOT {} {}\n",
					                   targets[0],
					                   targets[i]);
				}
				break;
			}
		} break;
		}

		return true;
	});
}

template<typename Network>
void write_quil(Network const& circ, const std::string& filename)
{
	std::ofstream out(filename.c_str(), std::ofstream::out);
	write_quil(circ, out);
}

}; // namespace tweedledum
