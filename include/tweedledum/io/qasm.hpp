/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate_lib.hpp"
#include "../gates/gate_base.hpp"
#include "../networks/io_id.hpp"
#include "../program.hpp"

#include <cassert>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <tweedledee/qasm/ast/visitor.hpp>
#include <tweedledee/qasm/qasm.hpp>

namespace tweedledee::qasm {

template<typename Program>
class tweedledum_visitor : public visitor_base<tweedledum_visitor<Program>> {
public:
	explicit tweedledum_visitor(Program& qprogram)
	    : qprogram_(qprogram)
	{}

	/* Containers */
	void visit_decl_gate(decl_gate* node)
	{
		// Ignore gate declarations for now
		(void) node;
		return;
	}

	std::string visit_expr_argument(expr_argument* node)
	{
		auto reg = static_cast<decl_register*>(node->register_decl());
		auto index = static_cast<expr_integer*>(node->index());
		if (index) {
			return fmt::format("{}_{}", reg->identifier(), index->evaluate());
		}
		return fmt::format("{}", reg->identifier());
	}

	void visit_stmt_gate(stmt_gate* node)
	{
		using namespace tweedledum;
		auto gate_id = static_cast<decl_gate*>(node->gate())->identifier();
		auto arguments_list = visit_list_any(static_cast<list_any*>(node->arguments()));
		if (gate_id == "cx") {
			qprogram_.network_.add_gate(gate::cx, arguments_list[0], arguments_list[1]);
		} else if (gate_id == "t") {
			qprogram_.network_.add_gate(gate::t, arguments_list[0]);
		} else if (gate_id == "tdg") {
			qprogram_.network_.add_gate(gate::t_dagger, arguments_list[0]);
		}
	}

	std::vector<std::string> visit_list_any(list_any* node)
	{
		std::vector<std::string> arguments;
		for (auto& child : *node) {
			assert(child.kind() == ast_node_kinds::expr_argument);
			arguments.emplace_back(visit_expr_argument(static_cast<expr_argument*>(&child)));
		}
		return arguments;
	}

	void visit_decl_register(decl_register* node)
	{
		std::string_view register_name = node->identifier();
		if (node->is_quantum()) {
			for (uint32_t i = 0u; i < node->size(); ++i) {
				qprogram_.network_.add_qubit(fmt::format("{}_{}", register_name, i));
			}
		} else {
			for (uint32_t i = 0u; i < node->size(); ++i) {
				qprogram_.network_.add_cbit(fmt::format("{}_{}", register_name, i));
			}
		}
	}

private:
	Program& qprogram_;
};
} // namespace tweedledee::qasm

namespace tweedledum {

/*! \brief Reads OPENQASM 2.0 format
 */
template<typename Program>
void read_qasm_from_file(std::string const& path, Program& qprogram)
{
	using namespace tweedledee::qasm;
	auto program_ast = read_from_file(path);
	if (program_ast) {
		ast_printer printer;
		printer.visit(*program_ast);

		tweedledum_visitor program_builder(qprogram);
		program_builder.visit(*program_ast);
	}
}

/*! \brief Writes network in OPENQASM 2.0 format into output stream
 *
 * An overloaded variant exists that writes the network into a file.
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
	if (network.num_cbits()) {
		os << fmt::format("creg c[{}];\n", network.num_cbits());
	}

	network.foreach_gate([&](auto const& node) {
		auto const& gate = node.gate;
		switch (gate.operation()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			assert(0);
			return true;

		case gate_lib::hadamard:
			gate.foreach_target([&](auto target) { os << fmt::format("h q[{}];\n", target); });
			break;

		case gate_lib::pauli_x:
			gate.foreach_target([&](auto target) { os << fmt::format("x q[{}];\n", target); });
			break;

		case gate_lib::pauli_z:
			gate.foreach_target([&](auto target) { os << fmt::format("z q[{}];\n", target); });
			break;

		case gate_lib::phase:
			gate.foreach_target([&](auto target) { os << fmt::format("s q[{}];\n", target); });
			break;

		case gate_lib::phase_dagger:
			gate.foreach_target([&](auto target) { os << fmt::format("sdg q[{}];\n", target); });
			break;

		case gate_lib::t:
			gate.foreach_target([&](auto target) { os << fmt::format("t q[{}];\n", target); });
			break;

		case gate_lib::t_dagger:
			gate.foreach_target([&](auto target) { os << fmt::format("tdg q[{}];\n", target); });
			break;

		case gate_lib::rotation_z:
			gate.foreach_target([&](auto target) {
				os << fmt::format("rz({}) q[{}];\n", gate.rotation_angle(), target);
			});
			break;

		case gate_lib::rotation_y:
			gate.foreach_target([&](auto target) {
				os << fmt::format("ry({}) q[{}];\n", gate.rotation_angle(), target);
			});
			break;

		case gate_lib::rotation_x:
			gate.foreach_target([&](auto target) {
				os << fmt::format("rx({}) q[{}];\n", gate.rotation_angle(), target);
			});
			break;

		case gate_lib::cx:
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

		case gate_lib::swap: {
			std::vector<io_id> targets;
			gate.foreach_target([&](auto target) {
				targets.push_back(target);
			});
			os << fmt::format("cx q[{}], q[{}];\n", targets[0], targets[1]);
			os << fmt::format("cx q[{}], q[{}];\n", targets[1], targets[0]);
			os << fmt::format("cx q[{}], q[{}];\n", targets[0], targets[1]);
		} break;

		case gate_lib::mcx: {
			std::vector<io_id> controls;
			std::vector<io_id> targets;
			gate.foreach_control([&](auto control) {
				if (control.is_complemented()) {
					os << fmt::format("x q[{}];\n", control.index());
				}
				controls.push_back(control.id()); 
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
