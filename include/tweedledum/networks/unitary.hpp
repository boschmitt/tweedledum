/*-------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*------------------------------------------------------------------------------------------------*/
#pragma once

#include "../gates/gate.hpp"
#include "storage.hpp"
#include "wire_id.hpp"

#include <cassert>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <complex>
#include <vector>

namespace tweedledum {

namespace matrices {

using complex_type = std::complex<double>;
constexpr double sqrt_1_2 = 0.707106781186547524401;

constexpr std::array<complex_type, 4> i = {{{1., 0.}, {0., 0.}, {0., 0.}, {1., 0.}}};
constexpr std::array<complex_type, 4> h = {{{sqrt_1_2, 0.}, {sqrt_1_2, 0.}, {sqrt_1_2, 0.}, {-sqrt_1_2, 0.}}};
constexpr std::array<complex_type, 4> x = {{{0., 0.}, {1., 0.}, {1., 0.}, {0., 0.}}};
constexpr std::array<complex_type, 4> y = {{{0., 0.}, {0., 1.}, {0., -1.}, {0., 0.}}};
constexpr std::array<complex_type, 4> z = {{{1., 0.}, {0., 0.}, {0., 0.}, {-1., 0.}}};
constexpr std::array<complex_type, 4> s = {{{1., 0.}, {0., 0.}, {0., 0.}, {0., 1.}}};
constexpr std::array<complex_type, 4> sdg = {{{1., 0.}, {0., 0.}, {0., 0.}, {0., -1.}}};
constexpr std::array<complex_type, 4> t = {{{1., 0.}, {0., 0.}, {0., 0.}, {sqrt_1_2, sqrt_1_2}}};
constexpr std::array<complex_type, 4> tdg = {{{1., 0.}, {0., 0.}, {0., 0.}, {sqrt_1_2, -sqrt_1_2}}};

inline std::array<complex_type, 4> r1(angle lambda)
{
	std::array<complex_type, 4> matrix = i;
	matrix.at(3) = std::exp(complex_type(0., lambda.numeric_value()));
	return matrix;
}

inline std::array<complex_type, 4> rx(angle theta)
{
	std::array<complex_type, 4> matrix = i;
	matrix.at(0) = {std::cos(theta.numeric_value() / 2.), 0.};
	matrix.at(1) = {0., -std::sin(theta.numeric_value() / 2.)};
	matrix.at(2) = {0., -std::sin(theta.numeric_value() / 2.)};
	matrix.at(3) = {std::cos(theta.numeric_value() / 2.), 0.};
	return matrix;
}

inline std::array<complex_type, 4> ry(angle theta)
{
	std::array<complex_type, 4> matrix = i;
	matrix.at(0) = {std::cos(theta.numeric_value() / 2.), 0.};
	matrix.at(1) = {std::sin(theta.numeric_value() / 2.), 0.};
	matrix.at(2) = {-std::sin(theta.numeric_value() / 2.), 0.};
	matrix.at(3) = {std::cos(theta.numeric_value() / 2.), 0.};
	return matrix;
}

inline std::array<complex_type, 4> rz(angle lambda)
{
	std::array<complex_type, 4> matrix = i;
	matrix.at(0) = std::exp(complex_type(0., -lambda.numeric_value()));
	matrix.at(3) = std::exp(complex_type(0., lambda.numeric_value()));
	return matrix;
}

inline std::array<complex_type, 4> u3(angle theta, angle phi, angle lambda)
{
	using namespace std::complex_literals;
	std::array<complex_type, 4> matrix = i;
	matrix.at(0) = std::cos(theta.numeric_value() / 2.);
	matrix.at(1) = std::exp(1.i * phi.numeric_value()) * std::sin(theta.numeric_value() / 2.);
	matrix.at(2) = -std::exp(1.i * lambda.numeric_value()) * std::sin(theta.numeric_value() / 2.);
	matrix.at(3) = std::exp(1.i * (phi.numeric_value() + lambda.numeric_value()))
	               * std::cos(theta.numeric_value() / 2.);
	return matrix;
}
} // namespace matrices

/*! \brief 
 *
 * The unitary matrix is vectorized. Column-major
 */
class unitary {
#pragma region Types and constructors
	using complex_type = std::complex<double>;

	struct dstrg_type {
		dstrg_type(uint32_t num_qubits, std::string_view name)
		    : num_rows(1 << num_qubits)
		    , name(name)
		    , matrix((1u << (2 * num_qubits)), {0., 0.})
		{
			for (uint32_t i = 0u; i < matrix.size(); i += num_rows + 1) {
				matrix.at(i) = {1., 0.};
			}
		}

		uint32_t num_rows;
		std::string name;
		std::vector<complex_type> matrix;
	};

public:
	using wstrg_type = wire::storage;

	// Initialized with identity
	explicit unitary(uint32_t num_qubits = 0u)
	    : data_(std::make_shared<dstrg_type>(num_qubits, "tweedledum_unitary"))
	    , wires_(std::make_shared<wstrg_type>())
	{
		for (uint32_t i = 0u; i < num_qubits; ++i) {
			std::string name = fmt::format("__dum_q{}", i);
			wires_->create_qubit(name, wire_modes::inout);
		}
	}

	template<typename Network>
	explicit unitary(Network network)
	    : unitary(network.num_qubits())
	{
		using op_type = typename Network::op_type;
		network.foreach_op([&](op_type const& op) {
			if (op.is_one_qubit()) {
				create_op(op, op.target());
			} else if (op.is_two_qubit()) {
				create_op(op, op.control(), op.target());
			} else {
				std::vector<wire_id> controls;
				std::vector<wire_id> targets;
				op.foreach_control([&](wire_id control) {
					controls.push_back(control);
				});
				op.foreach_target([&](wire_id target) {
					targets.push_back(target);
				});
				create_op(op, controls, targets);
			}
		});
	}
#pragma endregion

#pragma region Wires
private:
	void grow_unitary()
	{
		if (num_qubits() == 1u) {
			data_->matrix = {{1., 0.}, {0., 0.}, {0., 0.}, {1., 0.}};
			data_->num_rows = 2u;
			return;
		}
		std::vector<complex_type> new_matrix((1u << (2 * num_qubits())), {0., 0.});
		auto new_begin = new_matrix.begin();
		for (uint32_t i = 0u; i < 2u; ++i) {
			auto old_begin = data_->matrix.cbegin();
			auto old_end = data_->matrix.cend();
			while (old_begin != old_end) {
				new_begin = std::copy(old_begin, old_begin + data_->num_rows,
				                      new_begin);
				old_begin += data_->num_rows;
				new_begin += data_->num_rows;
			}
			new_begin += data_->num_rows;
		}
		data_->num_rows <<= 1u;
		data_->matrix = std::move(new_matrix);
	}

public:
	uint32_t num_wires() const
	{
		return wires_->num_wires();
	}

	uint32_t num_qubits() const
	{
		return wires_->num_qubits();
	}

	uint32_t num_cbits() const
	{
		return 0u;
	}

	wire_id create_qubit(std::string_view name, wire_modes mode = wire_modes::inout)
	{
		wire_id w_id = wires_->create_qubit(name, mode);
		grow_unitary();
		return w_id;
	}

	wire_id create_qubit(wire_modes mode = wire_modes::inout)
	{
		std::string name = fmt::format("__dum_q{}", num_qubits());
		return create_qubit(name, mode);
	}

	wire_id wire(std::string_view name) const
	{
		return wires_->wire(name);
	}

	std::string wire_name(wire_id w_id) const
	{
		return wires_->wire_name(w_id);
	}

	void wire_name(wire_id w_id, std::string_view new_name, bool rename = true)
	{
		wires_->wire_name(w_id, new_name, rename);
	}

	wire_modes wire_mode(wire_id w_id) const
	{
		return wires_->wire_mode(w_id);
	}

	void wire_mode(wire_id w_id, wire_modes new_mode)
	{
		wires_->wire_mode(w_id, new_mode);
	}
#pragma endregion

#pragma region Creating operations (using wire ids)
private:
	uint32_t first_idx(std::vector<uint32_t> const& qubits, uint32_t const k)
	{
		uint32_t lowbits;
		uint32_t result = k;
		for (uint32_t j = 0u; j < qubits.size(); ++j) {
			lowbits = result & ((1 << qubits.at(j)) - 1);
			result >>= qubits.at(j);
			result <<= qubits.at(j) + 1;
			result |= lowbits;
		}
		return result;
	}

	std::vector<uint32_t> indicies(std::vector<uint32_t> const& qubits,
	                               std::vector<uint32_t> const& qubits_sorted, uint32_t const k)
	{
		std::vector<uint32_t> result((1 << qubits.size()), 0u);
		result.at(0) = first_idx(qubits_sorted, k);
		for (uint32_t i = 0u; i < qubits.size(); ++i) {
			uint32_t const n = (1u << i);
			uint32_t const bit = (1u << qubits.at(i));
			for (size_t j = 0; j < n; j++) {
				result.at(n + j) = result.at(j) | bit;
			}
		}
		return result;
	}

	void apply_matrix(std::array<complex_type, 4> const& matrix, wire_id target)
	{
		uint32_t const k_end = (data_->matrix.size() >> 1u);
		std::vector<uint32_t> qubits(1, target);

		// This is implemented for a genral one-qubit unitary matrix.  There are ways to
		// specialize for diagonal and anti-diagonal matrices.  I will leave it to the
		// future. TODO: optmize.
		for (uint32_t k = 0u; k < k_end; ++k) {
			std::vector<uint32_t> const idx = indicies(qubits, qubits, k);
			complex_type const temp = data_->matrix.at(idx.at(0));
			data_->matrix.at(idx.at(0)) = matrix.at(0) * temp
			                              + matrix.at(2) * data_->matrix.at(idx.at(1));
			data_->matrix.at(idx.at(1)) = matrix.at(1) * temp
			                              + matrix.at(3) * data_->matrix.at(idx.at(1));
		}
	}

	void apply_ncx_matrix(std::vector<wire_id> const& controls, wire_id target)
	{
		std::vector<uint32_t> qubits(controls.begin(), controls.end());
		qubits.emplace_back(target);
		std::vector<uint32_t> qubits_sorted = qubits;
		std::sort(qubits_sorted.begin(), qubits_sorted.end());

		uint32_t const n_qubits = qubits.size();
		uint32_t const k_end = (data_->matrix.size() >> n_qubits);
		uint32_t const p0 = (1 << (n_qubits - 1)) - 1;
		uint32_t const p1 = (1 << n_qubits) - 1;
		for (uint32_t k = 0u; k < k_end; ++k) {
			std::vector<uint32_t> const idx = indicies(qubits, qubits_sorted, k);
			std::swap(data_->matrix.at(idx.at(p0)), data_->matrix.at(idx.at(p1)));
		}
	}

	void apply_ncy_matrix(std::vector<wire_id> const& controls, wire_id target)
	{
		std::vector<uint32_t> qubits(controls.begin(), controls.end());
		qubits.emplace_back(target);
		std::vector<uint32_t> qubits_sorted = qubits;
		std::sort(qubits_sorted.begin(), qubits_sorted.end());

		uint32_t const n_qubits = qubits.size();
		uint32_t const k_end = (data_->matrix.size() >> n_qubits);
		uint32_t const p0 = (1 << (n_qubits - 1)) - 1;
		uint32_t const p1 = (1 << n_qubits) - 1;
		for (uint32_t k = 0u; k < k_end; ++k) {
			std::vector<uint32_t> const idx = indicies(qubits, qubits_sorted, k);
			complex_type const temp = data_->matrix.at(idx.at(p0));
			data_->matrix.at(idx.at(p0)) = complex_type(0., -1.) * data_->matrix.at(idx.at(p1));
			data_->matrix.at(idx.at(p1)) = complex_type(0., 1.) * temp;
		}
	}

	void apply_ncr1_matrix(std::vector<wire_id> const& controls, wire_id target,
	                       complex_type phase)
	{
		std::vector<uint32_t> qubits(controls.begin(), controls.end());
		qubits.emplace_back(target);
		std::vector<uint32_t> qubits_sorted = qubits;
		std::sort(qubits_sorted.begin(), qubits_sorted.end());

		uint32_t const n_qubits = qubits.size();
		uint32_t const k_end = (data_->matrix.size() >> n_qubits);
		uint32_t const p1 = (1 << n_qubits) - 1;
		for (uint32_t k = 0u; k < k_end; ++k) {
			std::vector<uint32_t> const idx = indicies(qubits, qubits_sorted, k);
			data_->matrix.at(idx.at(p1)) *= phase;
		}
	}

public:
	void create_op(gate const& g, wire_id t)
	{
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::i:
			break;

		case gate_ids::h:
			apply_matrix(matrices::h, t);
			break;

		case gate_ids::x:
			apply_matrix(matrices::x, t);
			break;

		case gate_ids::y:
			apply_matrix(matrices::y, t);
			break;

		case gate_ids::z:
			apply_matrix(matrices::z, t);
			break;

		case gate_ids::s:
			apply_matrix(matrices::s, t);
			break;

		case gate_ids::sdg:
			apply_matrix(matrices::sdg, t);
			break;

		case gate_ids::t:
			apply_matrix(matrices::t, t);
			break;

		case gate_ids::tdg:
			apply_matrix(matrices::tdg, t);
			break;
		
		// Parameterisable gates
		case gate_ids::r1:
			apply_matrix(matrices::r1(g.rotation_angle()), t);
			break;

		case gate_ids::rx:
			apply_matrix(matrices::rx(g.rotation_angle()), t);
			break;

		case gate_ids::ry:
			apply_matrix(matrices::ry(g.rotation_angle()), t);
			break;

		case gate_ids::rz:
			apply_matrix(matrices::rz(g.rotation_angle()), t);
			break;

		case gate_ids::u3:
			apply_matrix(matrices::u3(g.theta(), g.phi(), g.lambda()), t);
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
			apply_ncx_matrix(std::vector<wire_id>({c}), t);
			break;

		case gate_ids::cy:
			apply_ncy_matrix(std::vector<wire_id>({c}), t);
			break;

		case gate_ids::cz:
			apply_ncr1_matrix(std::vector<wire_id>({c}), t, complex_type(-1., 0.));
			break;

		case gate_ids::swap:
			break;

		// Parameterisable gates
		case gate_ids::crx:
			break;

		case gate_ids::cry:
			break;

		case gate_ids::crz:
			break;

		default:
			break;
		}
	}

	void create_op(gate const& g, wire_id c0, wire_id c1, wire_id t)
	{
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::ncx:
			apply_ncx_matrix({c0, c1}, t);
			break;

		case gate_ids::ncy:
			apply_ncy_matrix({c0, c1}, t);
			break;

		case gate_ids::ncz:
			apply_ncr1_matrix({c0, c1}, t, complex_type(-1., 0.));
			break;

		// Parameterisable gates
		case gate_ids::ncrx:
			break;

		case gate_ids::ncry:
			break;

		case gate_ids::ncrz:
			break;

		default:
			break;
		}
	}

	void create_op(gate const& g, std::vector<wire_id> cs, std::vector<wire_id> ts)
	{
		switch (g.id()) {
		// Non-parameterisable gates
		case gate_ids::ncx:
			apply_ncx_matrix(cs, ts.at(0));
			break;

		case gate_ids::ncy:
			apply_ncy_matrix(cs, ts.at(0));
			break;

		case gate_ids::ncz:
			apply_ncr1_matrix(cs, ts.at(0), complex_type(-1., 0.));
			break;

		// Parameterisable gates
		case gate_ids::ncrx:
			break;

		case gate_ids::ncry:
			break;

		case gate_ids::ncrz:
			break;

		default:
			break;
		}
	}
#pragma endregion

#pragma region Creating operations (using wire names)
	void create_op(gate const& g, std::string_view target)
	{
		create_op(g, wire(target));
	}

	void create_op(gate const& g, std::string_view l0, std::string_view l1)
	{
		create_op(g, wire(l0), wire(l1));
	}

	void create_op(gate const& g, std::string_view c0, std::string_view c1, std::string_view t)
	{
		create_op(g, wire(c0), wire(c1), wire(t));
	}

	void create_op(gate const& g, std::vector<std::string> const& cs,
	               std::vector<std::string> const& ts)
	{
		std::vector<wire_id> controls;
		for (std::string_view control : cs) {
			controls.push_back(wire(control));
		}
		std::vector<wire_id> targets;
		for (std::string_view target : ts) {
			targets.push_back(wire(target));
		}
		create_op(g, controls, targets);
	}
#pragma endregion

#pragma region Comparator
	// rtol : Relative tolerance 
	// atol : Absolute tolerance 
	bool is_apprx_equal(unitary const& other, double rtol = 1e-05, double atol = 1e-08) const
	{
		assert(data_->matrix.size() == other.data_->matrix.size());
		uint32_t const end = data_->matrix.size();
		bool is_close = true;
		for (uint32_t i = 0u; i < end && is_close; ++i) {
			is_close &= std::abs(data_->matrix.at(i).real() - other.data_->matrix.at(i).real())
			            <= (atol + rtol * std::abs(other.data_->matrix.at(i).real()));
			is_close &= std::abs(data_->matrix.at(i).imag() - other.data_->matrix.at(i).imag())
			            <= (atol + rtol * std::abs(other.data_->matrix.at(i).imag()));
		}
		return is_close;
	}
#pragma endregion

#pragma region Debug
	// Threshold for rounding small values to zero when printing
	void print(double threshold = 1e-10) const
	{
		for (uint32_t i = 0u; i < data_->num_rows; ++i) {
			for (uint32_t j = 0; j < data_->matrix.size(); j += data_->num_rows) {
				double const re = std::real(data_->matrix.at(i + j));
				double const img = std::imag(data_->matrix.at(i + j));
				complex_type const entry((std::abs(re) < threshold ? 0. : re),
				                         (std::abs(img) < threshold ? 0. : img));
				fmt::print("{} ", entry);
			}
			fmt::print("\n");
		}
	}
#pragma endregion

private:
	std::shared_ptr<dstrg_type> data_;
	std::shared_ptr<wstrg_type> wires_;
	
};

} // namespace tweedledum
