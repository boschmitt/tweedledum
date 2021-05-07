/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "Lexer.h"
#include "Token.h"

#include <cctype>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace tweedledum::qasm {

bool Lexer::skip_whitespace(char const* current_pos_)
{
    if ((*current_pos_ == ' ') || (*current_pos_ == '\t')) {
        ++current_pos_;
        while ((*current_pos_ == ' ') || (*current_pos_ == '\t')) {
            ++current_pos_;
        }
        src_position_ = current_pos_;
        return true;
    }
    return false;
}

bool Lexer::skip_line_comment(char const* current_pos_)
{
    while (*current_pos_ != 0 && *current_pos_ != '\n' && *current_pos_ != '\r')
    {
        ++current_pos_;
    }
    src_position_ = ++current_pos_;
    return true;
}

Token Lexer::create_token(char const* token_end, Token::Kinds const kind)
{
    uint32_t token_len = token_end - src_position_;
    src_position_ = token_end;
    return Token(kind, current_location() - token_len, token_len,
      src_position_ - token_len);
}

Token Lexer::lex_numeric_constant(char const* current_pos_)
{
    // We have already matched [0-9]
    while (std::isdigit(*current_pos_)) {
        ++current_pos_;
    }
    if (*current_pos_ != '.') {
        return create_token(current_pos_, Token::Kinds::nninteger);
    }
    ++current_pos_;
    while (std::isdigit(*current_pos_)) {
        ++current_pos_;
    }
    if (*current_pos_ == 'e' || *current_pos_ == 'E') {
        ++current_pos_;
        if (*current_pos_ == '+' || *current_pos_ == '-') {
            ++current_pos_;
        }
        while (std::isdigit(*current_pos_)) {
            ++current_pos_;
        }
    }
    return create_token(current_pos_, Token::Kinds::real);
}

Token Lexer::lex_identifier(char const* current_pos_)
{
    // We have already matched [a-z$]
    while (std::isalpha(*current_pos_) || std::isdigit(*current_pos_)
           || *current_pos_ == '_')
    {
        ++current_pos_;
    }
    // Check if the identifier is a known keyword
    auto keyword = kw_tokens.find(std::string(src_position_, current_pos_));
    if (keyword != kw_tokens.end()) {
        return create_token(current_pos_, keyword->second);
    }
    // Check if the identifier is a known preprocessor keyword
    keyword = pp_tokens.find(std::string(src_position_, current_pos_));
    if (keyword != pp_tokens.end()) {
        return create_token(current_pos_, keyword->second);
    }
    return create_token(current_pos_, Token::Kinds::identifier);
}

Token Lexer::lex()
{
lex_next_token:
    skip_whitespace(src_position_);
    char const* current_pos_ = src_position_;
    // Read a character, advancing over it.
    Token::Kinds kind = Token::Kinds::unknown;
    uint32_t length = 1;
    char c = *current_pos_++;

    switch (c) {
    case 0:
        kind = Token::Kinds::eof;
        break;

    case '\r':
        if (*current_pos_ == '\n') {
            ++current_pos_;
        }
        // FALLTHROUGH
    case '\n':
        src_position_ = current_pos_;
        goto lex_next_token;

    case '/':
        if (*current_pos_ == '/') {
            skip_line_comment(current_pos_);
            goto lex_next_token;
        }
        kind = Token::Kinds::slash;
        break;

    // clang-format off
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return lex_numeric_constant(current_pos_);
        // clang-format on

    case 'C':
        if (*current_pos_ == 'X') {
            ++current_pos_;
            kind = Token::Kinds::kw_cx;
            length = 2;
        }
        break;

    case 'U':
        kind = Token::Kinds::kw_u;
        break;

    // clang-format off
    case 'O':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
        return lex_identifier(current_pos_);
        // clang-format on

    case '[':
        kind = Token::Kinds::l_square;
        break;

    case ']':
        kind = Token::Kinds::r_square;
        break;

    case '(':
        kind = Token::Kinds::l_paren;
        break;

    case ')':
        kind = Token::Kinds::r_paren;
        break;

    case '{':
        kind = Token::Kinds::l_brace;
        break;

    case '}':
        kind = Token::Kinds::r_brace;
        break;

    case '*':
        kind = Token::Kinds::star;
        break;

    case '+':
        kind = Token::Kinds::plus;
        break;

    case '-':
        if (*current_pos_ == '>') {
            ++current_pos_;
            kind = Token::Kinds::arrow;
            length = 2;
            break;
        }
        kind = Token::Kinds::minus;
        break;

    case '^':
        kind = Token::Kinds::caret;
        break;

    case ';':
        kind = Token::Kinds::semicolon;
        break;

    case '=':
        if (*current_pos_ == '=') {
            ++current_pos_;
            kind = Token::Kinds::equalequal;
            length = 2;
        }
        break;

    case ',':
        kind = Token::Kinds::comma;
        break;

    case '"':
        while (*current_pos_ != '"' && *current_pos_ != '\n'
               && *current_pos_ != '\r') {
            ++current_pos_;
        }
        if (*current_pos_ != '"') {
            std::cerr << "Unmatched \", strings must on the same line\n";
            break;
        }
        ++current_pos_;
        return create_token(current_pos_, Token::Kinds::string);

    default:
        return Token(Token::Kinds::error, current_location(), length, nullptr);
    }
    uint32_t location = current_location();
    src_position_ = current_pos_;
    return Token(kind, location, length, nullptr);
}

} // namespace tweedledum::qasm
