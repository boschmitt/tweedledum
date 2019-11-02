/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../base/source.hpp"
#include "../base/source_manager.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include "token_kinds.hpp"

#include <memory>
#include <vector>

namespace tweedledee {
namespace quil {

// This is the class able to handle include's. You see, lexers know only about
// tokens within a single source file.
class preprocessor {
	using LexerPtr = std::unique_ptr<lexer>;

	source_manager& source_manager_;
	diagnostic_engine& diagnostic_;

	std::vector<LexerPtr> lexer_stack_;
	// The current top of the stack that we're lexing from.
	LexerPtr current_lexer_ = nullptr;

public:
	preprocessor(source_manager& source_manager, diagnostic_engine& diagnostic)
	    : source_manager_(source_manager)
	    , diagnostic_(diagnostic)
	{}

	// FIXME: For now this is like this because the function to
	// open files does not accept 'string_view' as parameter
	void add_target_file(const std::string& file_path)
	{
		auto source = source_manager_.add_target_file(file_path);
		if (current_lexer_ != nullptr) {
			lexer_stack_.push_back(std::move(current_lexer_));
		}
		current_lexer_ = std::make_unique<lexer>(source->offset(), source->content());
	}

	void add_target_buffer(const std::string_view buffer)
	{
		auto source = source_manager_.add_target_buffer(buffer);
		if (current_lexer_ != nullptr) {
			lexer_stack_.push_back(std::move(current_lexer_));
		}
		current_lexer_ = std::make_unique<lexer>(source->offset(), source->content());
	}

	token next_token()
	{
		if (current_lexer_ == nullptr) {
			std::cerr << "No target to lex.\n";
			return {};
		}
		auto token = current_lexer_->next_token();
		if (token.kind == token_kinds::pp_include) {
			handle_include();
			token = current_lexer_->next_token();
		} else if (token.kind == token_kinds::eof) {
			if (not lexer_stack_.empty()) {
				current_lexer_ = std::move(lexer_stack_.back());
				lexer_stack_.pop_back();
				token = current_lexer_->next_token();
			} else {
				current_lexer_ = nullptr;
			}
		}
		return token;
	}

private:
	// The "include" tokens have just been read, read the file to be included
	// from the lexer, then include it!
	void handle_include()
	{
		auto token = current_lexer_->next_token();
		if (token.kind != token_kinds::string) {
			std::cerr << "Include must be followed by a file name\n";
		}
		std::string_view target = token;
		token = current_lexer_->next_token();
		if (token.kind != token_kinds::new_line) {
			std::cerr << "Missing a new_line\n";
		}
		add_target_file(std::string(target.substr(1, target.length() - 2)));
	}
};

} // namespace quil
} // namespace tweedledee
