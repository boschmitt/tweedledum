/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Token.h"
#include "tweedledum/Utils/Source.h"

#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace tweedledum::qasm {

/*! \brief
 *
 * lexer class provides a simple interface that turns a text buffer into a
 * stream of tokens.  This provides no support for file reading or buffering, or
 * buffering/seeking of tokens, only forward lexing is supported.
 *
 * The lexer doesn't return tokens for every character in the file, it skips
 * whitespace and comments.
 */
class Lexer {
public:
    // Create a new lexer object for the specified source. This lexer
    // assumes that the associated file buffer will outlive it, so it
    // doesn't take ownership of it, hence the naked pointer.
    Lexer(Source const* source)
        : source_(source)
        , src_position_(source_->cbegin())
    {}

    // Lex a token and consume it.
    Token next_token()
    {
        return lex();
    }

    Source const* source() const
    {
        return source_;
    }

private:
    // Return current token location.
    uint32_t current_location() const
    {
        return source_->offset() + (src_position_ - source_->cbegin());
    }

    // Skip over a series of whitespace characters.  Update `src_position_`
    // to point to the next non-whitespace character and return.
    bool skip_whitespace(char const* current_pos_);

    // We have just read the `//` characters from input. Skip until we find
    // the newline or EOF character thats terminate the comment.  Then
    // update `src_position_` and return.
    bool skip_line_comment(char const* current_pos_);

    // When we lex a identifier or a numeric constant token, the token is
    // formed by a span of chars starting at buffer_prt and going till
    // token_end.  This method takes that range and assigns it to the token
    // as its location and size.  It also update `src_position_`.
    Token create_token(char const* token_end, Token::Kinds const kind);

    // Match [0-9]*, we have already matched [0-9]
    Token lex_numeric_constant(char const* current_pos_);

    // Match [_A-Za-z0-9]*, we have already matched [a-z$]
    Token lex_identifier(char const* current_pos_);

    // Return the next token in the buffer. If this is the end of buffer, it
    // return the EOF token.
    Token lex();

    // Delete copy-constructor
    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;

    Source const* const source_;
    // Current pointer into source content. (Next character to be lex)
    char const* src_position_;
};

} // namespace tweedledum::qasm
