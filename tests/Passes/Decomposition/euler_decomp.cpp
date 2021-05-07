/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "tweedledum/Passes/Decomposition/euler_decomp.h"

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/All.h"
#include "tweedledum/Utils/Matrix.h"

#include "../check_unitary.h"

#include <catch.hpp>
#include <cmath>

using namespace tweedledum;

UMatrix2 create_matrix(double theta, double phi, double lambda)
{
    using namespace std::complex_literals;
    UMatrix2 matrix;
    matrix(0, 0) = std::cos(theta / 2.);
    matrix(0, 1) = -std::exp(1.i * lambda) * std::sin(theta / 2.);
    matrix(1, 0) = std::exp(1.i * phi) * std::sin(theta / 2.);
    matrix(1, 1) = std::exp(1.i * (phi + lambda)) * std::cos(theta / 2.);
    return matrix;
}

TEST_CASE("Euler decomp test cases", "[euler_decomp][decomp]")
{
    using namespace tweedledum;
    bool const upto_global_phase = true;
    Circuit original;
    Qubit q0 = original.create_qubit();
    UMatrix2 matrix = Op::H().matrix();
    original.apply_operator(Op::Unitary(matrix), {q0});

    Circuit decomposed = euler_decomp(original);
    CHECK(check_unitary(original, decomposed, upto_global_phase));

    double smallest = 1e-18;
    double factor = 3.2;
    double lambda = 0.9;
    double phi = 0.7;
    for (uint32_t i = 0; i < 22u; ++i) {
        Circuit original;
        Qubit q0 = original.create_qubit();
        UMatrix2 matrix =
          create_matrix(smallest * std::pow(factor, i), phi, lambda);
        original.apply_operator(Op::Unitary(matrix), {q0});
        Circuit decomposed = euler_decomp(original);
        CHECK(check_unitary(original, decomposed, upto_global_phase));
    }
}
