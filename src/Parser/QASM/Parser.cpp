/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "Parser.h"
#include "Token.h"
#include "tweedledum/Operators/Standard.h"

#include <memory>

namespace tweedledum::qasm {

bool Parser::parse(Circuit& circuit)
{
    parse_header();
    while (1) {
        switch (current_token_.kind()) {
        case Token::Kinds::eof:
            return true;

        case Token::Kinds::kw_creg:
            parse_creg(circuit);
            break;

        case Token::Kinds::kw_qreg:
            parse_qreg(circuit);
            break;

        case Token::Kinds::identifier:
        case Token::Kinds::kw_cx:
        case Token::Kinds::kw_measure:
        case Token::Kinds::kw_u:
            parse_qop(circuit);
            break;

        default:
            emit_error(current_token_.spelling());
            emit_error("expected a top-level entity.");
            return false;
        }
    }
}

void Parser::parse_header()
{
    consume_token();
    expect_and_consume_token(Token::Kinds::kw_openqasm);
    expect_and_consume_token(Token::Kinds::real);
    expect_and_consume_token(Token::Kinds::semicolon);
}

void Parser::parse_creg(Circuit& circuit)
{
    // If we get here, then 'creg' was matched
    consume_token();
    std::string_view name = expect_and_consume_token(Token::Kinds::identifier);
    expect_and_consume_token(Token::Kinds::l_square);
    uint32_t size = expect_and_consume_token(Token::Kinds::nninteger);
    expect_and_consume_token(Token::Kinds::r_square);
    expect_and_consume_token(Token::Kinds::semicolon);
    for (uint32_t i = 0u; i < size; ++i) {
        circuit.create_cbit(fmt::format("{}_{}", name, i));
    }
    return;
}

void Parser::parse_qreg(Circuit& circuit)
{
    // If we get here, then 'qreg' was matched
    consume_token();
    std::string_view name = expect_and_consume_token(Token::Kinds::identifier);
    expect_and_consume_token(Token::Kinds::l_square);
    uint32_t size = expect_and_consume_token(Token::Kinds::nninteger);
    expect_and_consume_token(Token::Kinds::r_square);
    expect_and_consume_token(Token::Kinds::semicolon);
    for (uint32_t i = 0u; i < size; ++i) {
        circuit.create_qubit(fmt::format("{}_{}", name, i));
    }
    return;
}

void Parser::parse_gate_statement(Circuit& circuit)
{
    // If we get here, then an identifier was matched
    std::string_view name = expect_and_consume_token(Token::Kinds::identifier);
    std::vector<double> parameters;
    if (try_and_consume_token(Token::Kinds::l_paren)) {
        if (!try_and_consume_token(Token::Kinds::r_paren)) {
            parameters.push_back(consume_parameter());
            expect_and_consume_token(Token::Kinds::r_paren);
        }
    }
    std::vector<Qubit> qubits;
    do {
        qubits.push_back(parse_argument());
        if (!try_and_consume_token(Token::Kinds::comma)) {
            break;
        }
    } while (1);
    expect_and_consume_token(Token::Kinds::semicolon);
    if (name == "x" || name == "cx") {
        circuit.apply_operator(Op::X(), qubits);
    } else if (name == "h") {
        circuit.apply_operator(Op::H(), qubits);
    } else if (name == "t") {
        circuit.apply_operator(Op::T(), qubits);
    } else if (name == "tdg") {
        circuit.apply_operator(Op::Tdg(), qubits);
    } else if (name == "rx") {
        circuit.apply_operator(Op::Rx(parameters.at(0)), qubits);
    } else if (name == "ry") {
        circuit.apply_operator(Op::Ry(parameters.at(0)), qubits);
    } else if (name == "rz") {
        circuit.apply_operator(Op::Rz(parameters.at(0)), qubits);
    } else if (name == "s") {
        circuit.apply_operator(Op::S(), qubits);
    } else if (name == "sdg") {
        circuit.apply_operator(Op::Sdg(), qubits);
    } else if (name == "sx") {
        circuit.apply_operator(Op::Sx(), qubits);
    } else if (name == "sxdg") {
        circuit.apply_operator(Op::Sxdg(), qubits);
    } else if (name == "y") {
        circuit.apply_operator(Op::Y(), qubits);
    } else if (name == "z" || name == "cz") {
        circuit.apply_operator(Op::Z(), qubits);
    } else {
        assert(0);
    }
    return;
}

void Parser::parse_qop(Circuit& circuit)
{
    switch (current_token_.kind()) {
    case Token::Kinds::kw_measure:
        break;

    case Token::Kinds::identifier:
        parse_gate_statement(circuit);
        break;

    case Token::Kinds::kw_cx:
        parse_cnot(circuit);
        break;

    case Token::Kinds::kw_u:
        parse_u(circuit);
        break;

    default:
        break;
    }
}

// TODO: This works only if there is only one quantum register!
Qubit Parser::parse_argument()
{
    expect_and_consume_token(Token::Kinds::identifier);
    if (!try_and_consume_token(Token::Kinds::l_square)) {
        // TODO
        return Qubit::invalid();
    }
    uint32_t idx = expect_and_consume_token(Token::Kinds::nninteger);
    expect_and_consume_token(Token::Kinds::r_square);
    return Qubit(idx);
}

void Parser::parse_cnot(Circuit& circuit)
{
    // If we get here 'CX' was matched
    consume_token();
    Qubit control = parse_argument();
    expect_and_consume_token(Token::Kinds::comma);
    Qubit target = parse_argument();
    expect_and_consume_token(Token::Kinds::semicolon);
    circuit.apply_operator(Op::X(), {control, target});
}

void Parser::parse_u(Circuit& circuit)
{
    // If we get here 'U' was matched
    consume_token();
    expect_and_consume_token(Token::Kinds::l_paren);
    double theta = consume_parameter();
    expect_and_consume_token(Token::Kinds::comma);
    double phi = consume_parameter();
    expect_and_consume_token(Token::Kinds::comma);
    double lambda = consume_parameter();
    expect_and_consume_token(Token::Kinds::r_paren);
    Qubit target = parse_argument();
    expect_and_consume_token(Token::Kinds::semicolon);
    circuit.apply_operator(Op::U(theta, phi, lambda), {target});
}

} // namespace tweedledum::qasm
