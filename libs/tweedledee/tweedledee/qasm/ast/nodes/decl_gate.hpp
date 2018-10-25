/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <string>

namespace tweedledee {
namespace qasm {

//
class decl_gate
    : public ast_node
    , public ast_node_container<decl_gate, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location, std::string_view identifier)
		    : statement_(new decl_gate(location, identifier))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			statement_->add_child(std::move(child));
		}

		decl_gate& get()
		{
			return *statement_;
		}

		std::unique_ptr<decl_gate> finish()
		{
			return std::move(statement_);
		}

	private:
		std::unique_ptr<decl_gate> statement_;
	};

	std::string_view identifier() const
	{
		return identifier_;
	}

private:
	decl_gate(std::uint32_t location, std::string_view identifier)
	    : ast_node(location)
	    , identifier_(identifier)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::decl_gate;
	}

	void do_print(std::ostream& out) const override
	{
		out << "decl_gate "
		    << "'" << identifier_ << "'\n";
	}

private:
	std::string identifier_;
};

} // namespace qasm
} // namespace tweedledee
