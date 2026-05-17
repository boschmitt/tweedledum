/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cstddef>
#include <cstdint>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <tweedledum/Utils/Matrix.h>

namespace tweedledum::python {

// Convert a 2D numpy bool array (any layout) to BMatrix.
inline BMatrix to_bmatrix(
    nanobind::ndarray<bool, nanobind::ndim<2>, nanobind::device::cpu> const& arr)
{
    auto const rows = static_cast<Eigen::Index>(arr.shape(0));
    auto const cols = static_cast<Eigen::Index>(arr.shape(1));
    BMatrix M(rows, cols);
    auto v = arr.view();
    for (Eigen::Index i = 0; i < rows; ++i) {
        for (Eigen::Index j = 0; j < cols; ++j) {
            M(i, j) = v(static_cast<size_t>(i), static_cast<size_t>(j));
        }
    }
    return M;
}

// Convert a BMatrix to a newly-allocated 2D numpy bool array (owned by Python).
inline nanobind::ndarray<nanobind::numpy, bool, nanobind::ndim<2>>
    from_bmatrix(BMatrix const& M)
{
    namespace nb = nanobind;
    auto const rows = static_cast<size_t>(M.rows());
    auto const cols = static_cast<size_t>(M.cols());
    auto* data = new bool[rows * cols];
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            data[i * cols + j] = static_cast<bool>(static_cast<uint32_t>(M(
                static_cast<Eigen::Index>(i), static_cast<Eigen::Index>(j))));
        }
    }
    nb::capsule owner(data, [](void* p) noexcept { delete[] static_cast<bool*>(p); });
    size_t const shape[2] = {rows, cols};
    return nb::ndarray<nb::numpy, bool, nb::ndim<2>>(data, 2, shape, owner);
}

} // namespace tweedledum::python
