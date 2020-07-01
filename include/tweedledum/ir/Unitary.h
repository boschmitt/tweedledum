/*------------------------------------------------------------------------------
| Part of tweedledum.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "GateLib.h"
#include "Operator.h"
#include "WireStorage.h"

#include <cstdint>
#include <complex>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string_view>
#include <vector>

namespace tweedledum {

class Unitary : public WireStorage {
	using Complex = std::complex<double>;
public:
	Unitary(std::string_view name) : name_(name), rows_(0u) {}

	WireRef create_qubit(std::string_view name)
	{
		WireRef wire = do_create_qubit(name);
		grow_unitary();
		return wire;
	}

	WireRef create_qubit()
	{
		std::string const name = fmt::format("__dum_q{}", num_qubits());
		return create_qubit(name);
	}

	WireRef request_ancilla()
	{
		if (free_ancillae_.empty()) {
			return create_qubit(
			    fmt::format("__dum_a{}", num_qubits()));
		} else {
			WireRef qubit = free_ancillae_.back();
			free_ancillae_.pop_back();
			return qubit;
		}
	}

	void release_ancilla(WireRef qubit)
	{
		free_ancillae_.push_back(qubit);
	}

	template<typename OptorType>
	void create_instruction(OptorType const& optor,
	    std::vector<WireRef> const& controls, WireRef target)
	{
		apply_matrix(to_matrix(optor), controls, target);
	}

	template<typename OptorType>
	void create_instruction(
	    OptorType const& optor, std::vector<WireRef> const& wires)
	{
		if (wires.size() == 1) {
			apply_matrix(to_matrix(optor), wires.back());
		} else {
			apply_matrix(to_matrix(optor), {wires.begin(), wires.end() - 1}, wires.back());
		}
	}

	static std::string_view kind()
	{
		return "unitary_matrix";
	}

	std::string_view name() const
	{
		return name_;
	}

	friend void to_json(nlohmann::json& j, Unitary const& circuit);

private:
	void grow_unitary()
	{
		if (num_qubits() == 1u) {
			data_ = {{1., 0.}, {0., 0.}, {0., 0.}, {1., 0.}};
			rows_ = 2u;
			return;
		}
		std::vector<Complex> new_matrix((1u << (2 * num_qubits())), {0., 0.});
		auto new_begin = new_matrix.begin();
		for (uint32_t i = 0u; i < 2u; ++i) {
			auto old_begin = data_.cbegin();
			auto old_end = data_.cend();
			while (old_begin != old_end) {
				new_begin = std::copy(old_begin, old_begin + rows_,
				                      new_begin);
				old_begin += rows_;
				new_begin += rows_;
			}
			new_begin += rows_;
		}
		rows_ <<= 1u;
		data_ = std::move(new_matrix);
	}

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

	void apply_matrix(std::array<Complex, 4> const& matrix, WireRef target)
	{
		uint32_t const k_end = (data_.size() >> 1u);
		std::vector<uint32_t> qubits(1, target);

		// This is implemented for a genral one-qubit unitary matrix. 
		// There are ways to specialize for diagonal and anti-diagonal
		// matrices.  I will leave it to the future.
		// TODO: optmize.
		for (uint32_t k = 0u; k < k_end; ++k) {
			std::vector<uint32_t> const idx
			    = indicies(qubits, qubits, k);
			Complex const temp = data_.at(idx.at(0));
			data_.at(idx.at(0))
			    = matrix.at(0) * temp
			      + matrix.at(2) * data_.at(idx.at(1));
			data_.at(idx.at(1))
			    = matrix.at(1) * temp
			      + matrix.at(3) * data_.at(idx.at(1));
		}
	}

	// Applies a general n-controlled matrix
	void apply_matrix(std::array<Complex, 4> const& matrix,
	    std::vector<WireRef> const& controls, WireRef target)
	{
		std::vector<uint32_t> qubits(controls.begin(), controls.end());
		qubits.emplace_back(target);
		std::vector<uint32_t> qubits_sorted = qubits;
		std::sort(qubits_sorted.begin(), qubits_sorted.end());

		uint32_t const n_qubits = qubits.size();
		uint32_t const k_end = (data_.size() >> n_qubits);
		uint32_t const p0 = (1 << (n_qubits - 1)) - 1;
		uint32_t const p1 = (1 << n_qubits) - 1;
		for (uint32_t k = 0u; k < k_end; ++k) {
			std::vector<uint32_t> const idx
			    = indicies(qubits, qubits_sorted, k);
			Complex const temp = data_.at(idx.at(p0));
			data_.at(idx.at(p0))
			    = matrix.at(0) * temp
			      + matrix.at(2) * data_.at(idx.at(p1));
			data_.at(idx.at(p1))
			    = matrix.at(1) * temp
			      + matrix.at(3) * data_.at(idx.at(p1));
		}
	}

	friend bool is_approx_equal(Unitary const& rhs, Unitary const& lhs,
	    double const rtol, double const atol);

	friend void print(Unitary const& u, std::ostream& os, uint32_t indent,
	    double const threshold);

	std::string const name_;
	uint32_t rows_;
	std::vector<Complex> data_;
	std::vector<WireRef> free_ancillae_; // Should this be here?!
};

// rtol : Relative tolerance
// atol : Absolute tolerance
inline bool is_approx_equal(Unitary const& rhs, Unitary const& lhs,
    double const rtol = 1e-05, double const atol = 1e-08)
{
	assert(rhs.data_.size() == lhs.data_.size());
	uint32_t const end = rhs.data_.size();
	bool is_close = true;
	for (uint32_t i = 0u; i < end && is_close; ++i) {
		is_close
		    &= std::abs(rhs.data_.at(i).real() - lhs.data_.at(i).real())
		       <= (atol + rtol * std::abs(lhs.data_.at(i).real()));
		is_close
		    &= std::abs(rhs.data_.at(i).imag() - lhs.data_.at(i).imag())
		       <= (atol + rtol * std::abs(lhs.data_.at(i).imag()));
	}
	return is_close;
}

// Threshold for rounding small values to zero when printing
inline void print(Unitary const& u, std::ostream& os, uint32_t indent,
    double const threshold = 1e-10)
{
	for (uint32_t i = 0u; i < u.rows_; ++i) {
		for (uint32_t j = 0; j < u.data_.size(); j += u.rows_) {
			double const re = std::real(u.data_.at(i + j));
			double const img = std::imag(u.data_.at(i + j));
			std::complex<double> const entry(
			    (std::abs(re) < threshold ? 0. : re),
			    (std::abs(img) < threshold ? 0. : img));
			fmt::print("{} ", entry);
		}
		fmt::print("\n");
	}
}

} // namespace tweedledum
