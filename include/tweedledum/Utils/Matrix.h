/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#pragma once

#include <cstdint>
#include <fmt/format.h>
#include <ostream>
#include <valarray>

namespace tweedledum {

// 2D, Row major
class Matrix {
    using nested_list = std::initializer_list<std::initializer_list<uint8_t>>;

public:
    static Matrix Identity(uint32_t size)
    {
        Matrix matrix(size, size);
        for (uint32_t i = 0; i < size; ++i) {
            matrix(i, i) = 1;
        }
        return matrix;
    }

    Matrix(uint32_t rows, uint32_t cols)
        : rows_(rows), cols_(cols), data_(rows * cols)
    {}

    Matrix(nested_list lists)
        : Matrix(lists.size(), lists.size() ? lists.begin()->size() : 0)
    {
        uint32_t row = 0;
        uint32_t col = 0;
        for (auto const& list : lists) {
            for (auto const& value : list) {
                data_[row * cols_ + col] = value;
                ++col;
            }
            col = 0;
            ++row;
        }
    }

    uint32_t num_columns() const
    {
        return cols_;
    }

    uint32_t num_rows() const
    {
        return rows_;
    }

    std::valarray<uint8_t> row(uint32_t i) const
    {
        return data_[std::slice(i * cols_, cols_, 1)];
    }

    std::slice_array<uint8_t> row(uint32_t i)
    {
        return data_[std::slice(i * cols_, cols_, 1)];
    }

    std::slice_array<uint8_t> column(uint32_t i)
    {
        return data_[std::slice(i, rows_, cols_)];
    }

    uint8_t& operator()(uint32_t row, uint32_t column)
    {
        return data_[row * cols_ + column];
    }

    uint8_t const& operator()(uint32_t row, uint32_t column) const
    {
        return data_[row * cols_ + column];
    }

    friend Matrix transpose(Matrix const& matrix);

private:
    uint32_t const rows_;
    uint32_t const cols_;
    std::valarray<uint8_t> data_;
};

inline Matrix transpose(Matrix const& matrix)
{
    // For now, I only need to deal with square matrices (:
    if (matrix.rows_ != matrix.cols_) {
        assert(0 && "Transpose is not implemented for the general case");
        std::abort();
    }
    Matrix result(matrix.cols_, matrix.rows_);
    for (uint32_t n = 0; n < matrix.cols_ * matrix.rows_; ++n) {
        uint32_t i = n / matrix.cols_;
        uint32_t j = n % matrix.cols_;
        result.data_[n] = matrix.data_[matrix.cols_ * j + i];
    }
    return result;
}

inline void print(Matrix const& matrix, std::ostream& os)
{
    for (uint32_t i = 0u; i < matrix.num_rows(); ++i) {
        for (uint32_t j = 0u; j < matrix.num_columns(); ++j) {
            os << fmt::format("{} ", matrix(i, j));
        }
        os << '\n';
    }
}

} // namespace tweedledum