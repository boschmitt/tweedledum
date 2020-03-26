/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "wire_id.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <complex>
#include <cstdint>
#include <fmt/format.h>
#include <string>
#include <tuple>
#include <xtensor-blas/xlinalg.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xfixed.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xvectorize.hpp>


using namespace std::complex_literals;

namespace tweedledum {

/*! \brief Class used to represent a quantum circuit as a list of operations.
 *
 */
class unitary {

#pragma region Types and constructors
private:
	using complex_type = std::complex<double>;
	using matrix_type = xt::xarray<complex_type>;
	using one_qubit_type = xt::xtensor_fixed<complex_type, xt::xshape<2, 2>>;

	one_qubit_type i_matrix = {{1., 0.}, {0., 1.}};
	one_qubit_type h_matrix = {{1.0 / std::sqrt( 2.0 ), 1.0 / std::sqrt( 2.0 )}, 
	                           {1.0 / std::sqrt( 2.0 ), -1.0 / std::sqrt( 2.0 )}};
	one_qubit_type x_matrix = {{0., 1.}, {1., 0.}};
	one_qubit_type y_matrix = {{0., -1.i}, {1i, 0.}};
	one_qubit_type z_matrix = {{1., 0.}, {0., -1.}};
	one_qubit_type s_matrix = {{1., 0.}, {0., 1i}};
	one_qubit_type sdg_matrix = {{1., 0.}, {0., -1i}};
	// Helper matrices
	one_qubit_type zc_matrix = {{1., 0.}, {0., 0.}};
	one_qubit_type oc_matrix = {{0., 0.}, {0., 1.}};

	one_qubit_type r1_matrix(angle rot)
	{
		return {{1., 0.}, {0., exp(1.i * rot.numeric_value())}};
	}

	one_qubit_type rx_matrix(angle rot)
	{
		return {{std::cos(rot.numeric_value()/2.0), -1.i*std::sin(rot.numeric_value()/2.0)}, 
		        {-1.i*std::sin(rot.numeric_value()/2.0), std::cos(rot.numeric_value()/2.0)}};
	}

	one_qubit_type ry_matrix(angle rot)
	{
		return {{std::cos(rot.numeric_value()/2.0), -std::sin(rot.numeric_value()/2.0)}, 
		        {std::sin(rot.numeric_value()/2.0), std::cos(rot.numeric_value()/2.0)}};
		return {{exp(-1.i * rot.numeric_value() / 2.0), 0.}, 
		        {0., exp(1.i * rot.numeric_value() / 2.0)}};
	}

	one_qubit_type rz_matrix(angle rot)
	{
		return {{exp(-1.i * rot.numeric_value() / 2.0), 0.}, 
		        {0., exp(1.i * rot.numeric_value() / 2.0)}};
	}

	one_qubit_type u3_matrix(double theta, double phi, double lambda)
	{
		return {{std::cos(theta / 2.0), -(std::exp(1.i * lambda) * std::sin(theta / 2.0))},
		        {(std::exp(1.i * phi) * std::sin(theta / 2.0)),
		         ((std::exp(1.i * (lambda + phi)) * std::cos(theta / 2.0)))}};
	}

public:
	explicit unitary(uint32_t n_qubits)
		: num_qubits_(n_qubits)
		, unitary_matrix_(create_identity(1 << num_qubits()))
		, qubit_buffers_(num_qubits(), i_matrix)
		, has_buffered_gates_(num_qubits(), 0u)
	{}

	template<typename Network>
	explicit unitary(Network const& network)
		: num_qubits_(network.num_qubits())
		, unitary_matrix_(create_identity(1 << num_qubits()))
		, qubit_buffers_(num_qubits(), i_matrix)
		, has_buffered_gates_(num_qubits(), 0u)
	{
		using op_type = typename Network::op_type;
		network.foreach_op([&](op_type const& op) {
			if (op.is_one_qubit()) {
				create_op(op, op.target());
			} else if (op.is_two_qubit()) {
				create_op(op, op.control(), op.target());
			}
		});
	}
#pragma endregion

#pragma region Properties
	uint32_t num_qubits() const
	{
		return num_qubits_;
	}
#pragma endregion

#pragma region Wires
	wire_id wire(uint32_t index) const
	{
		return wire_id(index, /* is_qubit */ true);
	}
#pragma endregion

#pragma region Creating operations (using wire ids)
private:
	template<typename M>
	matrix_type merge_gates(std::vector<M> const& matrices)
	{
		matrix_type result = matrices.at(0);
		for (auto i = 1u; i < matrices.size(); ++i) {
			result = xt::linalg::kron(result, matrices[i]);
		}
		return result;
	}

	matrix_type create_identity(uint32_t dimension)
	{
		matrix_type matrix(std::vector<std::size_t>{dimension, dimension});
		for (auto i = 0u; i < dimension; ++i) {
			matrix[{i, i}] = 1.0;
		}
		return matrix;
	}

	// Pads the gate with identity
	matrix_type pad(matrix_type const& matrix, uint32_t from, uint32_t to)
	{
		std::vector<matrix_type> temp_gates;
		if (to + 1u < num_qubits_) {
			temp_gates.push_back(create_identity(1 << (num_qubits_ - to - 1u)));
		}
		temp_gates.push_back(matrix);
		if (from > 0u) {
			temp_gates.push_back(create_identity(1 << from));
		}
		return merge_gates(temp_gates);
	}

	void apply_matrix(matrix_type const& matrix, uint32_t t)
	{
		if (has_buffered_gates_.at(t)) {
			qubit_buffers_.at(t) = xt::linalg::dot(matrix, qubit_buffers_.at(t));
		} else {
			qubit_buffers_.at(t) = matrix;
		}
		has_buffered_gates_.at(t) += 1;
	}

	void apply_matrix(matrix_type const& matrix, uint32_t w0, uint32_t w1)
	{
		apply_buffered_gates();
		uint32_t act = std::abs(static_cast<int>(w1 - w0));
		if (w0 < w1) {
			auto gate = xt::linalg::kron(create_identity(1 << act), zc_matrix)
			            + merge_gates<matrix_type>(
			                {matrix, create_identity(1 << (act - 1)), oc_matrix});
			apply_matrix(pad(gate, w0, w1));
		} else {
			auto gate = xt::linalg::kron(zc_matrix, create_identity(1 << act))
			            + merge_gates<matrix_type>(
			                {oc_matrix, create_identity(1 << (act - 1)), matrix});
			apply_matrix(pad(gate, w1, w0));
		}
	}

	void apply_matrix(matrix_type const& matrix)
	{
		unitary_matrix_ = xt::linalg::dot(matrix, unitary_matrix_);
	}

	bool has_buffered_gates() const
	{
		for (uint32_t i = 0; i < has_buffered_gates_.size(); ++i) {
			if (has_buffered_gates_.at(i)) {
				return true;
			}
		}
		return false;
	}

	void apply_buffered_gates()
	{
		if (!has_buffered_gates()) {
			return;
		}
		matrix_type new_gate = has_buffered_gates_.back() ? qubit_buffers_.back() : i_matrix;
		for (uint32_t i = qubit_buffers_.size() - 1; i-- > 0;) {
			if (has_buffered_gates_.at(i) == 0u) {
				new_gate = xt::linalg::kron(new_gate, i_matrix);
				continue;
			}
			new_gate = xt::linalg::kron(new_gate, qubit_buffers_.at(i));
		}
		unitary_matrix_ = xt::linalg::dot(new_gate, unitary_matrix_);
		std::fill(has_buffered_gates_.begin(), has_buffered_gates_.end(), 0u);
	}

public:
	void create_op(gate const& g, wire_id t)
	{
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::i:
			break;

		case gate_ids::h:
			apply_matrix(h_matrix, t);
			break;

		case gate_ids::x:
			apply_matrix(x_matrix, t);
			break;

		case gate_ids::y:
			apply_matrix(y_matrix, t);
			break;

		case gate_ids::z:
			apply_matrix(z_matrix, t);
			break;

		case gate_ids::s:
			apply_matrix(s_matrix, t);
			break;

		case gate_ids::sdg:
			apply_matrix(sdg_matrix, t);
			break;

		case gate_ids::t:
			apply_matrix(r1_matrix(sym_angle::pi_quarter), t);
			break;

		case gate_ids::tdg:
			apply_matrix(r1_matrix(-sym_angle::pi_quarter), t);
			break;
		
		// Parameterisable gates
		case gate_ids::r1:
			apply_matrix(r1_matrix(g.rotation_angle()), t);
			break;

		case gate_ids::rx:
			apply_matrix(rx_matrix(g.rotation_angle()), t);
			break;

		case gate_ids::ry:
			apply_matrix(ry_matrix(g.rotation_angle()), t);
			break;

		case gate_ids::rz:
			apply_matrix(rz_matrix(g.rotation_angle()), t);
			break;

		case gate_ids::u3:
			apply_matrix(u3_matrix(g.theta().numeric_value(), g.phi().numeric_value(),
			                       g.lambda().numeric_value()),
			             t);
			break;

		default:
			break;
		}
	}

	void create_op(gate const& g, wire_id c, wire_id t)
	{
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::cx:
			apply_matrix(x_matrix, c, t);
			break;

		case gate_ids::cy:
			apply_matrix(y_matrix, c.id(), t.id());
			break;

		case gate_ids::cz:
			apply_matrix(z_matrix, c, t);
			break;

		case gate_ids::swap:
			break;

		// Parameterisable gates
		case gate_ids::crx:
			apply_matrix(rx_matrix(g.rotation_angle()), c, t);
			break;

		case gate_ids::cry:
			apply_matrix(ry_matrix(g.rotation_angle()), c, t);
			break;

		case gate_ids::crz:
			apply_matrix(rz_matrix(g.rotation_angle()), c, t);
			break;

		default:
			break;
		}
	}
#pragma endregion

#pragma region Creating operations (using wire labels)
	// void create_op(gate const& g, std::string const& target)
	// {
	// 	return create_op(g, wire(target));
	// }

	// void create_op(gate const& g, std::string const& l0, std::string const& l1)
	// {
	// 	return create_op(g, wire(l0), wire(l1));
	// }

	// void create_op(gate const& g, std::string const& c0, std::string const& c1,
	//                std::string const& t)
	// {
	// 	return create_op(g, wire(c0), wire(c1), wire(t));
	// }

	// void create_op(gate const& g, std::vector<std::string> const& c_labels,
	//                std::vector<std::string> const& t_labels)
	// {
	// 	std::vector<wire_id> controls;
	// 	for (std::string const& control : c_labels) {
	// 		controls.push_back(wire(control));
	// 	}
	// 	std::vector<wire_id> targets;
	// 	for (std::string const& target : t_labels) {
	// 		targets.push_back(wire(target));
	// 	}
	// 	return create_op(g, controls, targets);
	// }
#pragma endregion

#pragma region Iterators
#pragma endregion

#pragma region Comparator
public:
	bool is_apprx_equal(unitary& other, double rtol = 1e-05, double atol = 1e-08)
	{
		this->apply_buffered_gates();
		other.apply_buffered_gates();
		return xt::allclose(unitary_matrix_, other.unitary_matrix_, rtol, atol);
	}
#pragma endregion

	void print()
	{
		apply_buffered_gates();
		std::cout << xt::print_options::line_width(600) << unitary_matrix_ << '\n';
	}

private:
	uint32_t num_qubits_;
	matrix_type unitary_matrix_;
	std::vector<one_qubit_type> qubit_buffers_;
	std::vector<uint32_t> has_buffered_gates_;
};

} // namespace tweedledum
