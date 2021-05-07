/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "PPLexer.h"
#include "Token.h"
#include "tweedledum/IR/Circuit.h"
#include "tweedledum/Utils/SourceManager.h"

#include <memory>

namespace tweedledum::qasm {

/*! \brief
 */
class Parser {
public:
    Parser(SourceManager& source_manager)
        : source_manager_(source_manager)
        , pp_lexer_(source_manager_)
    {}

    bool parse(Circuit& circuit);

private:
    // Consume the current token 'current_token_' and lex the next one.
    // Returns the location of the consumed token.
    uint32_t consume_token()
    {
        prev_token_location_ = current_token_.location();
        current_token_ = pp_lexer_.next_token();
        return prev_token_location_;
    }

    // The parser expects that the current token is of 'expected' kind.  If
    // it is not, it emits a diagnostic, puts the parser in a error state
    // and returns the current_token_. Otherwise consumes the token and
    // returns it.
    Token expect_and_consume_token(Token::Kinds const expected)
    {
        if (!current_token_.is(expected)) {
            // Should emit some error.
            return current_token_;
        }
        Token return_token = current_token_;
        consume_token();
        return return_token;
    }

    // The parser try to see if the current token is of 'expected' kind.  If
    // it is not, returns false.  Otherwise consumes the token and returns
    // true.
    bool try_and_consume_token(Token::Kinds const expected)
    {
        if (!current_token_.is(expected)) {
            return false;
        }
        consume_token();
        return true;
    }

    void emit_error(std::string_view message) const
    {
        fmt::print("[error] {} {}\n",
          source_manager_.location_str(current_token_.location()), message);
    }

    double consume_parameter()
    {
        bool is_minus = try_and_consume_token(Token::Kinds::minus);
        double result = expect_and_consume_token(Token::Kinds::real);
        if (is_minus) {
            return -result;
        }
        return result;
    }

    /*! \brief Parse OpenQASM file header */
    void parse_header();

    void parse_creg(Circuit& circuit);

    void parse_qreg(Circuit& circuit);

    void parse_gate_statement(Circuit& circuit);

    void parse_qop(Circuit& circuit);

    /*! \brief Parse an argument (<argument>) */
    // <argument> = <id>
    //            | <id> [ <nninteger> ]
    Qubit parse_argument();

    void parse_cnot(Circuit& circuit);

    void parse_u(Circuit& circuit);

    // Delete copy-constructor
    Parser(Parser const&) = delete;
    Parser& operator=(Parser const&) = delete;

    SourceManager& source_manager_;
    PPLexer pp_lexer_;

    // The current token we are peeking.
    Token current_token_;

    // The location of the token we previously consumed.  This is used for
    // diagnostics in which we expected to see a token following another
    // token (e.g., the ';' at the end of a statement).
    uint32_t prev_token_location_;
};

} // namespace tweedledum::qasm
