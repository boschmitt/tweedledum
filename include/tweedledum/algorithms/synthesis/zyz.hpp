/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/io_id.hpp"
#include "../../gates/gate_base.hpp"
#include "../../gates/gate_lib.hpp"
#include "../../utils/angle.hpp"

#include <complex>
#include <xtensor-blas/xlinalg.hpp>
#include <xtensor/xfixed.hpp>

namespace tweedledum {

using single_qubit_unitary = xt::xtensor_fixed<std::complex<double>, xt::xshape<2, 2>>;

static constexpr auto th_prec = 1e-10;

// implementation inspired by euler_angles_1q in Qiskit Terra
std::array<double, 3> zyz_decomposition(single_qubit_unitary const& matrix)
{
	const auto phase = 1.0 / std::sqrt(xt::linalg::det(matrix));
	const auto mat_norm = phase * matrix;
	const auto theta = std::abs(mat_norm(0, 0)) > th_prec ?
	                       2.0 * std::acos(std::abs(mat_norm(0, 0))) :
	                       2.0 * std::asin(std::abs(mat_norm(1, 0)));
	const auto phase11 = std::abs(std::cos(theta / 2.0)) > th_prec ?
	                         mat_norm(1, 1) / std::cos(theta / 2.0) :
	                         0.0;
	const auto phase10 = std::abs(std::sin(theta / 2.0)) > th_prec ?
	                         mat_norm(1, 0) / std::sin(theta / 2.0) :
	                         0.0;
	const auto phiplambda = 2.0 * std::atan2(phase11.imag(), phase11.real());
	const auto phimlambda = 2.0 * std::atan2(phase10.imag(), phase10.real());

	auto phi = 0.0;
	auto lambda = 0.0;

	if (std::abs(mat_norm(0, 0)) > th_prec && std::abs(mat_norm(1, 0)) > th_prec) {
		phi = (phiplambda + phimlambda) / 2.0;
		lambda = (phiplambda - phimlambda) / 2.0;
	} else {
		lambda = std::abs(mat_norm(0, 0)) < th_prec ? -phimlambda : phiplambda;
	}

	return {theta, phi, lambda};
}

template<class Network>
void add_single_qubit_unitary(Network& network, io_id const& target,
                              single_qubit_unitary const& matrix)
{
	const auto [theta, phi, lambda] = zyz_decomposition(matrix);
	network.add_gate(gate_base(gate_lib::u3, theta, phi, lambda), target);
}

} // namespace tweedledum
