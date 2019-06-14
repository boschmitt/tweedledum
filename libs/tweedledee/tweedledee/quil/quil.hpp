/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../base/source_manager.hpp"
#include "ast/ast.hpp"
#include "parser.hpp"
#include "preprocessor.hpp"
#include "semantic.hpp"
#include "token.hpp"
#include "token_kinds.hpp"

#include <string>

namespace tweedledee::quil {

inline std::unique_ptr<program> quil_read_from_file(std::string const& path)
{
	source_manager source_manager;
	preprocessor pp_lexer(source_manager);
	semantic semantic;
	parser parser(pp_lexer, semantic, source_manager);

	pp_lexer.add_target_file(path);
	auto success = parser.parse();
	if (success) {
		std::cout << "Valid Quil =)\n";
	} else {
		std::cout << "Invalid Quil =(\n";
	}
	return semantic.finish();
}

inline std::unique_ptr<program> quil_read_from_buffer(std::string const& buffer)
{
	source_manager source_manager;
	preprocessor pp_lexer(source_manager);
	semantic semantic;
	parser parser(pp_lexer, semantic, source_manager);

	pp_lexer.add_target_buffer(buffer);
	auto success = parser.parse();
	if (success) {
		std::cout << "Valid Quil =)\n";
	} else {
		std::cout << "Invalid Quil =(\n";
	}
	return semantic.finish();
}

} // namespace tweedledee::quil
