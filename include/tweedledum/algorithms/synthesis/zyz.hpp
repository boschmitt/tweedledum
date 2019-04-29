/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-------------------------------------------------------------------------------------------------*/
#pragma once

#include "../../networks/io_id.hpp"
#include "../../networks/netlist.hpp"
#include "../../utils/angle.hpp"

#include <complex>
#include <xtensor-blas/xlinalg.hpp>
#include <xtensor/xfixed.hpp>

namespace tweedledum {

using single_qubit_unitary = xt::xtensor_fixed<std::complex<double>, xt::xshape<2, 2>>;

static constexpr auto th_prec = 1e-10;

// implementation inspired by euler_angles_1q in Qiskit Terra
std::array<double, 3> zyz_decomposition(single_qubit_unitary const& mat)
{
	const auto phase = 1.0 / sqrt(xt::linalg::det(mat));
	const auto mat_norm = phase * mat;
	const auto theta = abs(mat_norm(0, 0)) > th_prec ? 2.0 * acos(abs(mat_norm(0, 0))) :
	                                                   2.0 * asin(abs(mat_norm(1, 0)));
	const auto phase11 = abs(cos(theta / 2.0)) > th_prec ? mat_norm(1, 1) / cos(theta / 2.0) :
	                                                       0.0;
	const auto phase10 = abs(sin(theta / 2.0)) > th_prec ? mat_norm(1, 0) / sin(theta / 2.0) :
	                                                       0.0;
	const auto phiplambda = 2.0 * atan2(phase11.imag(), phase11.real());
	const auto phimlambda = 2.0 * atan2(phase10.imag(), phase10.real());

	auto phi = 0.0;
	auto lambda = 0.0;

	if (abs(mat_norm(0, 0)) > th_prec && abs(mat_norm(1, 0)) > th_prec) {
		phi = (phiplambda + phimlambda) / 2.0;
		lambda = (phiplambda - phimlambda) / 2.0;
	} else {
		lambda = abs(mat_norm(0, 0)) < th_prec ? -phimlambda : phiplambda;
	}

	return {theta, phi, lambda};
}

template<class Network>
void add_single_qubit_unitary(Network& net, io_id const& target, single_qubit_unitary const& mat)
{
	const auto [theta, phi, lambda] = zyz_decomposition(mat);
	net.add_gate(gate_base(gate_set::u3, theta, phi, lambda), target);
}

} // namespace tweedledum
