/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "../networks/wire_id.hpp"

#include <cassert>
#include <fmt/format.h>
#include <fstream>
#include <string>
#include <vector>
#include <tweedledee/qasm/ast/visitor.hpp>
#include <tweedledee/qasm/qasm.hpp>

namespace tweedledee::qasm {

template<typename Network>
class tweedledum_visitor : public visitor_base<tweedledum_visitor<Network>> {
public:
	explicit tweedledum_visitor(Network& network)
	    : network_(network)
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
		auto gate_ids = static_cast<decl_gate*>(node->gate())->identifier();
		auto arguments_list = visit_list_any(static_cast<list_any*>(node->arguments()));
		auto parameters = node->parameters();
		if (gate_ids == "id") {
			network_.create_op(gate_lib::i, arguments_list[0]);
		} else if (gate_ids == "h") {
			network_.create_op(gate_lib::h, arguments_list[0]); 
		} else if (gate_ids == "x") {
			network_.create_op(gate_lib::x, arguments_list[0]); 
		} else if (gate_ids == "y") {
			network_.create_op(gate_lib::y, arguments_list[0]); 
		} else if (gate_ids == "z") {
			network_.create_op(gate_lib::z, arguments_list[0]); 
		} else if (gate_ids == "s") {
			network_.create_op(gate_lib::s, arguments_list[0]); 
		} else if (gate_ids == "sdg") {
			network_.create_op(gate_lib::sdg, arguments_list[0]); 
		} else if (gate_ids == "t") {
			network_.create_op(gate_lib::t, arguments_list[0]); 
		} else if (gate_ids == "tdg") {
			network_.create_op(gate_lib::tdg, arguments_list[0]); 
		} else if (gate_ids == "cx") {
			network_.create_op(gate_lib::cx, arguments_list[0], arguments_list[1]);
		} else if (gate_ids == "cy") {
			network_.create_op(gate_lib::cy, arguments_list[0], arguments_list[1]);
		} else if (gate_ids == "cz") {
			network_.create_op(gate_lib::cz, arguments_list[0], arguments_list[1]);
		} else if (gate_ids == "swap") {
			network_.create_op(gate_lib::swap, arguments_list[0], arguments_list[1]);
		} else if (gate_ids == "ccx") {
			network_.create_op(gate_lib::ncx, arguments_list[0], arguments_list[1],
			                   arguments_list[2]);
		} else if (gate_ids == "rz") {
			// FIXME: this is a hack! I need to properly implement expression evaluation
			assert(parameters != nullptr);
			auto parameter = &(*(static_cast<list_exps*>(parameters)->begin()));
			double angle = evaluate(parameter);
			network_.create_op(gate_lib::rz(angle), arguments_list[0]);
		} else {
			fmt::print("Unrecognized gate: {}\n", gate_ids);
			assert(0);
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
				network_.create_qubit(fmt::format("{}_{}", register_name, i));
			}
		} else {
			for (uint32_t i = 0u; i < node->size(); ++i) {
				network_.create_cbit(fmt::format("{}_{}", register_name, i));
			}
		}
	}

private:
	double evaluate(ast_node* node) const
	{
		switch (node->kind()) {
		case ast_node_kinds::expr_integer:
			return static_cast<expr_integer*>(node)->evaluate();

		case ast_node_kinds::expr_real:
			return static_cast<expr_real*>(node)->evaluate();

		case ast_node_kinds::expr_unary_op:
			if (static_cast<expr_unary_op*>(node)->op() == unary_ops::minus) {
				return -evaluate(static_cast<expr_unary_op*>(node)->operand());
			}

		default:
			// The time has come, implement a better way to evaluate expression!
			std::abort();
		}
	}

private:
	Network& network_;
};
} // namespace tweedledee::qasm

namespace tweedledum {

/*! \brief Reads OPENQASM 2.0 format
 */
template<typename Network>
Network read_qasm_from_buffer(std::string const& buffer)
{
	using namespace tweedledee::qasm;
	Network network;
	auto program_ast = read_from_buffer(buffer);
	if (program_ast) {
		tweedledum_visitor network_builder(network);
		network_builder.visit(*program_ast);
	}
	return network;
}

/*! \brief Reads OPENQASM 2.0 format
 */
template<typename Network>
Network read_qasm_from_file(std::string const& path)
{
	using namespace tweedledee::qasm;
	Network network;
	auto program_ast = read_from_file(path);
	if (program_ast) {
		// ast_printer printer;
		// printer.visit(*program_ast);

		tweedledum_visitor network_builder(network);
		network_builder.visit(*program_ast);
	}
	return network;
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
	using op_type = typename Network::op_type;
	// header
	os << "OPENQASM 2.0;\n";
	os << "include \"qelib1.inc\";\n";
	os << fmt::format("qreg q[{}];\n", network.num_qubits());
	if (network.num_cbits()) {
		os << fmt::format("creg c[{}];\n", network.num_cbits());
	}

	network.foreach_op([&](op_type const& op) {
		switch (op.id()) {
		default:
			std::cerr << "[w] unsupported gate type\n";
			return true;
		// Non-parameterisable gates
		case gate_ids::i:
			os << fmt::format("id q[{}];\n", op.target().id());
			break;

		case gate_ids::h:
			os << fmt::format("h q[{}];\n", op.target().id());
			break;

		case gate_ids::x:
			os << fmt::format("x q[{}];\n", op.target().id());
			break;

		case gate_ids::y:
			os << fmt::format("y q[{}];\n", op.target().id());
			break;

		case gate_ids::z:
			os << fmt::format("z q[{}];\n", op.target().id());
			break;

		case gate_ids::s:
			os << fmt::format("s q[{}];\n", op.target().id());
			break;

		case gate_ids::sdg:
			os << fmt::format("sdg q[{}];\n", op.target().id());
			break;

		case gate_ids::t:
			os << fmt::format("t q[{}];\n", op.target().id());
			break;

		case gate_ids::tdg:
			os << fmt::format("tdg q[{}];\n", op.target().id());
			break;

		case gate_ids::cx:
			os << fmt::format("cx q[{}], q[{}];\n", op.control().id(),
			                  op.target().id());
			break;

		case gate_ids::cy:
			os << fmt::format("cy q[{}], q[{}];\n", op.control().id(),
			                  op.target().id());
			break;

		case gate_ids::cz:
			os << fmt::format("cz q[{}], q[{}];\n", op.control().id(),
			                  op.target().id());
			break;

		case gate_ids::swap:
			os << fmt::format("swap q[{}], q[{}];\n", op.target(0u).id(),
			                  op.target(1u).id());
			break;

		case gate_ids::ncx:
			os << fmt::format("ccx q[{}], q[{}], q[{}];\n", op.control(0u),
			                  op.control(1u).id(), op.target());
			break;
		// Parameterisable gates
		case gate_ids::r1:
			os << fmt::format("u1({}) q[{}];\n", op.rotation_angle(), op.target());
			break;

		case gate_ids::rx:
			os << fmt::format("rx({}) q[{}];\n", op.rotation_angle(), op.target());
			break;

		case gate_ids::ry:
			os << fmt::format("ry({}) q[{}];\n", op.rotation_angle(), op.target());
			break;

		case gate_ids::rz:
			os << fmt::format("rz({}) q[{}];\n", op.rotation_angle(), op.target());
			break;
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
