/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <complex>
#include <Eigen/Dense>

namespace tweedledum {

using Complex = std::complex<double>;

// Unitary matrix is Column-Major!
using UMatrix = Eigen::Matrix<Complex, Eigen::Dynamic, Eigen::Dynamic>;
using UMatrix2 = Eigen::Matrix<Complex, 2, 2>;
using UMatrix4 = Eigen::Matrix<Complex, 4, 4>;

} // namespace tweedledum