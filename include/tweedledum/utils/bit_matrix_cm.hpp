/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "detail/foreach.hpp"
#include "detail/bit_matrix.hpp"
#include "dynamic_bitset.hpp"

#include <cassert>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

namespace tweedledum {

// Forward declaration
template<class MatrixType>
class bit_matrix_view;

/*! \brief Column-major matrix of bits */
template<class WordType = uint32_t>
class bit_matrix_cm final {
	friend class bit_matrix_view<bit_matrix_cm<WordType>>;

public:
	using word_type = WordType;
	using row_type = dynamic_bitset<WordType>;
	using column_type = dynamic_bitset<WordType>;
	using size_type = std::size_t;
	using container_type = detail::bit_matrix<WordType>;

#pragma region Constructors
	bit_matrix_cm(size_type num_rows)
	    : storage_(num_rows)
	{}

	bit_matrix_cm(size_type num_rows, size_type num_columns)
	    : storage_(num_rows, num_columns)
	{}

	template<typename ValueType>
	bit_matrix_cm(size_type num_rows, std::vector<ValueType> const& columns)
	    : storage_(num_rows, columns)
	{}
#pragma endregion

#pragma region Element access
	constexpr auto at(size_type row_index, size_type column_index) const
	{
		return column(column_index)[row_index];
	}

	constexpr auto at(size_type row_index, size_type column_index)
	{
		return column(column_index)[row_index];
	}

	constexpr auto const& column(size_type index) const
	{
		return storage_.line(index);
	}

	constexpr auto& column(size_type index)
	{
		return storage_.line(index);
	}

	constexpr auto row(size_type row_index) const
	{
		row_type value(num_columns(), 0);
		for (size_type column_index = 0; column_index < num_columns(); ++column_index) {
			value[column_index] = column(column_index)[row_index];
		}
		return value;
	}
#pragma endregion

#pragma region Iterators
	template<typename Fn>
	void foreach_column(Fn&& fn) const
	{
		detail::foreach_element(storage_.lines_.cbegin(), storage_.lines_.cend(), fn, 0);
	}

	template<typename Fn>
	void foreach_column(Fn&& fn)
	{
		detail::foreach_element(storage_.lines_.begin(), storage_.lines_.end(), fn, 0);
	}

	// TODO
	template<typename Fn>
	void foreach_row(Fn&& fn) const;

	// TODO
	template<typename Fn>
	void foreach_row(Fn&& fn);
#pragma endregion

#pragma region Capacity
	constexpr auto size() const
	{
		return std::make_pair(num_rows(), num_columns());
	}

	constexpr auto num_rows() const
	{
		return storage_.num_bits_per_line();
	}

	constexpr auto num_columns() const
	{
		return storage_.num_lines();
	}

	constexpr auto empty() const
	{
		return storage_.empty();
	}
#pragma endregion

#pragma region Modifiers
	constexpr void push_back_column(column_type const& column)
	{
		assert(column.size() == num_rows());
		storage_.push_back_line(column);
	}
#pragma endregion

#pragma region Debug
	void print(std::ostream& out = std::cout) const
	{
		for (size_type row_index = 0; row_index < num_rows(); ++row_index) {
			auto row = this->row(row_index);
			for (auto bit : row) {
				out << fmt::format("{} ", int(bit));
			}
			out << fmt::format("\n");
		}
	}
#pragma endregion
private:
	container_type storage_;
};

} // namespace tweedledum