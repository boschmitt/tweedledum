/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../ast_node.hpp"
#include "../ast_node_kinds.hpp"

#include <memory>
#include <rang/rang.hpp>

namespace tweedledee {
namespace quil {

//
class matrix
    : public ast_node
    , public ast_node_container<matrix, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location)
		    : statement_(new matrix(location))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			statement_->add_child(std::move(child));
		}

		matrix& get()
		{
			return *statement_;
		}

		std::unique_ptr<matrix> finish()
		{
			return std::move(statement_);
		}

	private:
		std::unique_ptr<matrix> statement_;
	};

private:
	matrix(std::uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::matrix;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << fg::green << "matrix" << fg::reset;
	}
};

class matrix_row
    : public ast_node
    , public ast_node_container<matrix_row, ast_node> {
public:
	class builder {
	public:
		explicit builder(std::uint32_t location)
		    : statement_(new matrix_row(location))
		{}

		void add_child(std::unique_ptr<ast_node> child)
		{
			statement_->add_child(std::move(child));
		}

		matrix_row& get()
		{
			return *statement_;
		}

		std::unique_ptr<matrix_row> finish()
		{
			return std::move(statement_);
		}

	private:
		std::unique_ptr<matrix_row> statement_;
	};

private:
	matrix_row(std::uint32_t location)
	    : ast_node(location)
	{}

	ast_node_kinds do_get_kind() const override
	{
		return ast_node_kinds::matrix_row;
	}

	void do_print(std::ostream& out) const override
	{
		using namespace rang;
		out << fg::green << "matrix_row" << fg::reset;
	}
};

} // namespace quil
} // namespace tweedledee
