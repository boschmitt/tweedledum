/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt, Mathias Soeken
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_set.hpp"
#include "../networks/qubit.hpp"

#include <cassert>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>
#include <tweedledee/qasm/qasm.hpp>
#include <tweedledee/qasm/ast/visitor.hpp>

namespace tweedledum {
#pragma region utility functions(for readability)
void node_printer(tweedledee::qasm::ast_node const& node)
{
	using namespace tweedledee::qasm;
	switch (node.kind()) {
	case ast_node_kinds::decl_gate:
		fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "decl_gate ");
		fmt::print(fg(fmt::color::cyan), "{}",
		           static_cast<decl_gate const&>(node).identifier());
		break;

	case ast_node_kinds::decl_param:
		fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "decl_param ");
		fmt::print(fg(fmt::color::cyan), "{}",
		           static_cast<decl_param const&>(node).identifier());
		break;

	case ast_node_kinds::decl_register:
		fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "decl_register ");
		fmt::print(fg(fmt::color::cyan), "{}",
		           static_cast<decl_register const&>(node).identifier());
		fmt::print(fg(fmt::color::green), " '{}:{}'",
		           static_cast<decl_register const&>(node).type() == register_type::quantum ?
		               "Quantum" :
		               "Classical",
		           static_cast<decl_register const&>(node).size());
		break;

	case ast_node_kinds::expr_binary_op:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "expr_binary_op ");
		fmt::print("{}", static_cast<expr_binary_op const&>(node).op());
		break;

	case ast_node_kinds::expr_decl_ref:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "expr_decl_ref ");
		node_printer(*static_cast<expr_decl_ref const&>(node).declaration());
		break;

	case ast_node_kinds::expr_integer:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "expr_integer ");
		fmt::print(fg(fmt::color::cyan), "{}",
		           static_cast<expr_integer const&>(node).evaluate());
		break;

	case ast_node_kinds::expr_real:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "expr_real ");
		fmt::print(fg(fmt::color::cyan), "{}", static_cast<expr_real const&>(node).value());
		break;

	case ast_node_kinds::expr_reg_idx_ref:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "expr_reg_idx_ref ");
		break;

	case ast_node_kinds::expr_unary_op:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "expr_unary_op ");
		fmt::print(fg(fmt::color::cyan), "{}",
		           static_cast<expr_unary_op const&>(node).name());
		break;

	case ast_node_kinds::stmt_cnot:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "stmt_cnot");
		break;

	case ast_node_kinds::stmt_gate:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "stmt_gate");
		break;

	case ast_node_kinds::stmt_unitary:
		fmt::print(fg(fmt::terminal_color::bright_magenta) | fmt::emphasis::bold,
		           "stmt_unitary");
		break;

	case ast_node_kinds::program:
	default:
		break;
	}
}

void print_ast(std::ostream& out, const tweedledee::qasm::program& program)
{
	using namespace tweedledee::qasm;
	out << "AST for :\n";
	std::string prefix;
	visit(program, [&](const ast_node& node, visitor_info info) {
		if (node.kind() == ast_node_kinds::program)
			return true;
		else if (info == visitor_info::container_end) {
			prefix.pop_back();
			prefix.pop_back();
		} else {
			out << prefix;
			if (info == visitor_info::container_begin)
				prefix += "| ";
			out << "|-";
			node_printer(node);
			out << '\n';
		}
		return true;
	});
}
#pragma endregion

/*! \brief Reads OPENQASM 2.0 format
 */
void read_qasm_from_file(std::string const& path)
{
	auto program_ast = tweedledee::qasm::read_from_file(path);
	if (program_ast) {
		print_ast(std::cout, *program_ast);
	}
}

/*! \brief Writes network in OPENQASM 2.0 format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
 *
 * **Required gate functions:**
 * - `foreach_control`
 * - `foreach_target`
 * - `op`
 * 
 * **Required network functions:**
 * - `foreach_cnode`
 * - `num_qubits`
 *
 * \param network A quantum network
 * \param os Output stream
 */
template<typename Network>
void write_qasm(Network const& network, std::ostream& os)
{
	// header
	os << "OPENQASM 2.0;\n";
	os << "include \"qelib1.inc\";\n";
	os << fmt::format("qreg q[{}];\n", network.num_qubits());
	os << fmt::format("creg c[{}];\n", network.num_qubits());

	network.foreach_cgate([&](auto const& node) {
		auto const& gate = node.gate;
		switch (gate.operation()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			assert(0);
			return true;

		case gate_set::hadamard:
			gate.foreach_target([&](auto target) { os << fmt::format("h q[{}];\n", target); });
			break;

		case gate_set::pauli_x:
			gate.foreach_target([&](auto target) { os << fmt::format("x q[{}];\n", target); });
			break;

		case gate_set::pauli_z:
			gate.foreach_target([&](auto target) { os << fmt::format("z q[{}];\n", target); });
			break;

		case gate_set::phase:
			gate.foreach_target([&](auto target) { os << fmt::format("s q[{}];\n", target); });
			break;

		case gate_set::phase_dagger:
			gate.foreach_target([&](auto target) { os << fmt::format("sdg q[{}];\n", target); });
			break;

		case gate_set::t:
			gate.foreach_target([&](auto target) { os << fmt::format("t q[{}];\n", target); });
			break;

		case gate_set::t_dagger:
			gate.foreach_target([&](auto target) { os << fmt::format("tdg q[{}];\n", target); });
			break;

		case gate_set::rotation_z:
			gate.foreach_target([&](auto target) {
				os << fmt::format("rz({:.17f}) q[{}];\n", gate.rotation_angle().numeric_value(), target);
			});
			break;
		case gate_set::rotation_y:
			gate.foreach_target([&](auto target) {
				os << fmt::format("ry({:.17f}) q[{}];\n", gate.rotation_angle().numeric_value(), target);
			});
			break;
		case gate_set::rotation_x:
			gate.foreach_target([&](auto target) {
				os << fmt::format("rx({:.17f}) q[{}];\n", gate.rotation_angle().numeric_value(), target);
			});
			break;

		case gate_set::cx:
			gate.foreach_control([&](auto control) {
				if (control.is_complemented()) {
					os << fmt::format("x q[{}];\n", control.index());
				}
				gate.foreach_target([&](auto target) {
					os << fmt::format("cx q[{}], q[{}];\n", control.index(), target);
				});
				if (control.is_complemented()) {
					os << fmt::format("x q[{}];\n", control.index());
				}
			});
			break;

		case gate_set::swap: {
			std::vector<qubit_id> targets;
			gate.foreach_target([&](auto target) {
				targets.push_back(target);
			});
			os << fmt::format("cx q[{}], q[{}];\n", targets[0], targets[1]);
			os << fmt::format("cx q[{}], q[{}];\n", targets[1], targets[0]);
			os << fmt::format("cx q[{}], q[{}];\n", targets[0], targets[1]);
		} break;

		case gate_set::mcx: {
			std::vector<qubit_id> controls;
			std::vector<qubit_id> targets;
			gate.foreach_control([&](auto control) {
				if (control.is_complemented()) {
					os << fmt::format("x q[{}];\n", control.index());
				}
				controls.push_back(control.index()); 
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
				for (auto q : targets) {
					os << fmt::format("x q[{}];\n", q);
				}
				break;

			case 1u:
				for (auto q : targets) {
					os << fmt::format("cx q[{}],q[{}];\n", controls[0], q);
				}
				break;

			case 2u:
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("cx q[{}], q[{}];\n", targets[0],
					                   targets[i]);
				}
				os << fmt::format("ccx q[{}], q[{}], q[{}];\n", controls[0],
				                   controls[1], targets[0]);
				for (auto i = 1u; i < targets.size(); ++i) {
					os << fmt::format("cx q[{}], q[{}];\n", targets[0],
					                   targets[i]);
				}
				break;
			}
			gate.foreach_control([&](auto control) {
				if (control.is_complemented()) {
					os << fmt::format("x q[{}];\n", control.index());
				}
			});
		} break;
		}
		return true;
	});
}

/*! \brief Writes network in OPENQASM 2.0 format into a file
 *
 * **Required gate functions:**
 * - `foreach_control`
 * - `foreach_target`
 * - `op`
 * 
 * **Required network functions:**
 * - `num_qubits`
 * - `foreach_cnode`
 *
 * \param network A quantum network
 * \param filename Filename
 */
template<typename Network>
void write_qasm(Network const& network, std::string const& filename)
{
	std::ofstream os(filename.c_str(), std::ofstream::out);
	write_qasm(network, os);
}

} // namespace tweedledum
