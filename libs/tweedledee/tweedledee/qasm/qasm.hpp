/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#include "../base/source_manager.hpp"
#include "../base/diagnostic.hpp"
#include "ast/ast.hpp"
#include "parser.hpp"
#include "preprocessor.hpp"
#include "semantic.hpp"
#include "token.hpp"
#include "token_kinds.hpp"

#include <string>
#include <memory>

namespace tweedledee::qasm {

inline std::unique_ptr<program> read_from_file(std::string const& path)
{
	source_manager source_manager;
	diagnostic_engine diagnostic;
	preprocessor pp_lexer(source_manager, diagnostic);
	semantic semantic(source_manager, diagnostic);
	parser parser(pp_lexer, semantic, source_manager, diagnostic);

	pp_lexer.add_target_file(path);
	auto success = parser.parse();
	if (success) {
		std::cout << "Valid QASM =)\n";
		return semantic.finish();
	} else {
		std::cout << "Invalid QASM =(\n";
		return nullptr;
	}
}

inline std::unique_ptr<program> read_from_buffer(std::string const& buffer)
{
	source_manager source_manager;
	diagnostic_engine diagnostic;
	preprocessor pp_lexer(source_manager, diagnostic);
	semantic semantic(source_manager, diagnostic);
	parser parser(pp_lexer, semantic, source_manager, diagnostic);

	pp_lexer.add_target_buffer(buffer);
	auto success = parser.parse();
	if (success) {
		std::cout << "Valid QASM =)\n";
		return semantic.finish();
	} else {
		std::cout << "Invalid QASM =(\n";
		return nullptr;
	}
}

} // namespace tweedledee::qasm
