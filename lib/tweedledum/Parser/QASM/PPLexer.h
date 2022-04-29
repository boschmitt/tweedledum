/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Lexer.h"
#include "Token.h"
#include "tweedledum/Utils/Source.h"
#include "tweedledum/Utils/SourceManager.h"

#include <filesystem>
#include <fmt/format.h>
#include <memory>
#include <vector>

namespace tweedledum::qasm {

/*! \brief Pre-processor Lexer class
 *
 * This is the class able to handle includes.  You see, lexers know only about
 * tokens within a single source file.
 *
 */
class PPLexer {
public:
    PPLexer(SourceManager& source_manager)
        : source_manager_(source_manager)
        , current_lexer_(nullptr)
    {
        Source const* source = source_manager_.main_source();
        if (source != nullptr) {
            current_lexer_ = std::make_unique<Lexer>(source);
        }
    }

    bool add_target_file(std::string_view path)
    {
        Source const* source = source_manager_.add_file(path);
        if (source == nullptr) {
            return false;
        }
        if (current_lexer_ != nullptr) {
            lexer_stack_.push_back(std::move(current_lexer_));
        }
        current_lexer_ = std::make_unique<Lexer>(source);
        return true;
    }

    void add_target_buffer(std::string_view buffer)
    {
        Source const* source = source_manager_.add_buffer(buffer);
        if (current_lexer_ != nullptr) {
            lexer_stack_.push_back(std::move(current_lexer_));
        }
        current_lexer_ = std::make_unique<Lexer>(source);
    }

    Token next_token()
    {
        if (current_lexer_ == nullptr) {
            std::cerr << "No target to lex.\n";
            return Token(Token::Kinds::error, 0, 0, nullptr);
        }
        Token tok = current_lexer_->next_token();
        if (tok.is(Token::Kinds::pp_include)) {
            handle_include();
            tok = current_lexer_->next_token();
        } else if (tok.is(Token::Kinds::eof)) {
            if (!lexer_stack_.empty()) {
                current_lexer_ = std::move(lexer_stack_.back());
                lexer_stack_.pop_back();
                tok = current_lexer_->next_token();
            } else {
                current_lexer_ = nullptr;
            }
        }
        return tok;
    }

private:
    // TODO
    void handle_include()
    {
        Token token = current_lexer_->next_token();
        if (!token.is(Token::Kinds::string)) {
            std::cerr << "Include must be followed by a file name\n";
            return;
        }
        token = current_lexer_->next_token();
        if (!token.is(Token::Kinds::semicolon)) {
            std::cerr << "Missing a ';'\n";
        }
    }

    SourceManager& source_manager_;
    std::vector<std::unique_ptr<Lexer>> lexer_stack_;
    std::unique_ptr<Lexer> current_lexer_;
};

} // namespace tweedledum::qasm
