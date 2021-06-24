/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../IR/Instruction.h"
#include "../../Utils/Matrix.h"

#include <cassert>
#include <fmt/format.h>
#include <optional>
#include <string_view>

namespace tweedledum::Op {

class Unitary {
public:
    static constexpr std::string_view kind()
    {
        return "ext.unitary";
    }

    Unitary(UMatrix const& unitary)
        : matrix_(unitary)
    {}

    Unitary(UMatrix&& unitary)
        : matrix_(unitary)
    {}

    UMatrix const& matrix() const
    {
        return matrix_;
    }

    uint32_t num_targets() const
    {
        // FIXME: rows are guaranteed to be a power of 2, do this with a
        // a dedicated function?
        return std::log2(matrix_.rows());
    }

    bool operator==(Unitary const& other) const
    {
        return matrix_ == other.matrix_;
    }

    friend double global_phase(Unitary const& unitary);

private:
    UMatrix matrix_;
};

inline double global_phase(Unitary const& unitary)
{
    Complex const phase = 1. / std::sqrt(unitary.matrix_.determinant());
    double angle = -std::arg(phase);
    return angle;
}

inline bool is_approx_equal(
  double lhs, double rhs, double const rtol = 1e-05, double const atol = 1e-08)
{
    return std::abs(rhs - lhs) <= (atol + rtol * std::abs(lhs));
}

// rtol : Relative tolerance
// atol : Absolute tolerance
inline bool is_approx_equal(Unitary const& lhs, Unitary const& rhs,
  bool up_to_global_phase = false, double const rtol = 1e-05,
  double const atol = 1e-08)
{
    assert(rhs.matrix().size() == lhs.matrix().size());
    bool is_close = true;
    uint32_t const end = rhs.matrix().size();
    auto const* r_data = rhs.matrix().data();
    auto const* l_data = lhs.matrix().data();
    if (!up_to_global_phase) {
        for (uint32_t i = 0u; i < end && is_close; ++i) {
            is_close &=
              is_approx_equal(l_data[i].real(), r_data[i].real(), rtol, atol);
            is_close &=
              is_approx_equal(l_data[i].imag(), r_data[i].imag(), rtol, atol);
        }
        return is_close;
    }
    auto const l_phase = std::exp(Complex(0., global_phase(lhs)));
    auto const r_phase = std::exp(Complex(0., global_phase(rhs)));
    for (uint32_t i = 0u; i < end && is_close; ++i) {
        Complex l = l_phase == Complex(0, 0) ? l_data[i] : l_data[i] / l_phase;
        Complex r = r_phase == Complex(0, 0) ? r_data[i] : r_data[i] / r_phase;
        is_close &= is_approx_equal(l.real(), r.real(), rtol, atol);
        is_close &= is_approx_equal(l.imag(), r.imag(), rtol, atol);
    }
    return is_close;
}

class UnitaryBuilder {
public:
    UnitaryBuilder(uint32_t const num_qubits, double const& phase = 0.0)
        : global_phase_(phase)
        , matrix_(UMatrix::Identity((1 << num_qubits), (1 << num_qubits)))
    {}

    template<typename OpT>
    void apply_operator(OpT&& optor, std::vector<Qubit> const& qubits)
    {
        std::vector<uint32_t> const temp_qubits(qubits.begin(), qubits.end());
        apply_operator(std::forward<OpT>(optor), temp_qubits);
    }

    template<typename OpT>
    void apply_operator(OpT&& optor, std::vector<uint32_t> const& qubits)
    {
        if (qubits.size() == 1u) {
            apply_matrix(optor.matrix(), qubits);
            return;
        }
        if constexpr (has_num_targets_v<OpT>) {
            switch (optor.num_targets()) {
            case 1u:
                apply_matrix_nc(optor.matrix(), qubits);
                break;

            case 2u:
                apply_matrix_nt<2>(optor.matrix(), qubits);
                break;

            default:
                assert(0);
                break;
            }
        } else {
            apply_matrix_nc(optor.matrix(), qubits);
        }
    }

    void apply_operator(
      Instruction const& inst, std::vector<uint32_t> const& qubits)
    {
        auto u_matrix = inst.matrix();
        if (!u_matrix) {
            fmt::print("Error: unitary matrix not defined");
            return;
        }

        if (qubits.size() == 1) {
            apply_matrix(u_matrix.value(), qubits);
            return;
        }
        switch (inst.num_targets()) {
        case 1u:
            apply_matrix_nc(u_matrix.value(), qubits);
            break;

        case 2u:
            // assert(0);
            apply_matrix_nt<2>(u_matrix.value(), qubits);
            break;

        default:
            assert(0);
            break;
        }
    }

    Unitary finished()
    {
        if (global_phase_ != 0.0) {
            matrix_ *= std::exp(Complex(0., global_phase_));
        }
        return Unitary(matrix_);
    }

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

    void apply_matrix(UMatrix const& matrix, std::vector<uint32_t> qubits)
    {
        uint32_t const k_end = (matrix_.size() >> 1u);

        // This is implemented for a general one-qubit unitary matrix.
        // There are ways to specialize for diagonal and anti-diagonal
        // matrices.  I will leave it to the future.
        // TODO: optmize.
        for (uint32_t k = 0u; k < k_end; ++k) {
            std::vector<uint32_t> const idx = indicies(qubits, qubits, k);
            Complex const cache = matrix_.data()[idx.at(0)];
            matrix_.data()[idx.at(0)] =
              matrix.data()[0] * cache
              + matrix.data()[2] * matrix_.data()[idx.at(1)];
            matrix_.data()[idx.at(1)] =
              matrix.data()[1] * cache
              + matrix.data()[3] * matrix_.data()[idx.at(1)];
        }
    }

    // Applies a general n-controlled 2x2 unitary matrix
    void apply_matrix_nc(
      UMatrix const& matrix, std::vector<uint32_t> const& qubits)
    {
        std::vector<uint32_t> qubits_sorted = qubits;
        std::sort(qubits_sorted.begin(), qubits_sorted.end());

        uint32_t const n_qubits = qubits.size();
        uint32_t const k_end = (matrix_.size() >> n_qubits);
        uint32_t const p0 = (1 << (n_qubits - 1)) - 1;
        uint32_t const p1 = (1 << n_qubits) - 1;

        auto* data_ = matrix_.data();
        for (uint32_t k = 0u; k < k_end; ++k) {
            std::vector<uint32_t> const idx =
              indicies(qubits, qubits_sorted, k);
            Complex const cache = data_[idx.at(p0)];
            data_[idx.at(p0)] =
              matrix.data()[0] * cache + matrix.data()[2] * data_[idx.at(p1)];
            data_[idx.at(p1)] =
              matrix.data()[1] * cache + matrix.data()[3] * data_[idx.at(p1)];
        }
    }

    template<uint32_t N>
    void apply_matrix_nt(
      UMatrix const& matrix, std::vector<uint32_t> const& qubits)
    {
        constexpr uint32_t k_dim = (1u << N);
        std::vector<uint32_t> qubits_sorted = qubits;
        std::sort(qubits_sorted.begin(), qubits_sorted.end());

        uint32_t const n_qubits = qubits.size();
        uint32_t const k_end = (matrix_.size() >> n_qubits);

        auto* data_ = matrix_.data();
        for (uint32_t k = 0u; k < k_end; ++k) {
            std::vector<uint32_t> const idx =
              indicies(qubits, qubits_sorted, k);
            std::array<Complex, k_dim> cache;
            for (size_t i = 0; i < k_dim; i++) {
                cache[i] = data_[idx.at(i)];
                data_[idx.at(i)] = 0.;
            }
            for (size_t i = 0; i < k_dim; i++) {
                for (size_t j = 0; j < k_dim; j++) {
                    data_[idx.at(i)] += matrix.data()[i + k_dim * j] * cache[j];
                }
            }
        }
    }

    double const global_phase_;
    UMatrix matrix_;
};

} // namespace tweedledum::Op
