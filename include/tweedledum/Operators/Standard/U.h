/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/Matrix.h"

#include <cmath>
#include <string_view>

namespace tweedledum::Op {

class U {
public:
    static constexpr std::string_view kind()
    {
        return "std.u";
    }

    U(double theta, double phi, double lambda)
        : theta_(theta)
        , phi_(phi)
        , lambda_(lambda)
    {}

    U adjoint() const
    {
        return U(-theta_, -phi_, -lambda_);
    }

    UMatrix2 const matrix() const
    {
        using namespace std::complex_literals;
        return (UMatrix2() << std::cos(theta_ / 2.),
          std::exp(1.i * phi_) * std::sin(theta_ / 2.),
          -std::exp(1.i * lambda_) * std::sin(theta_ / 2.),
          std::exp(1.i * (phi_ + lambda_)) * std::cos(theta_ / 2.))
          .finished();
    }

    bool operator==(U const& other) const
    {
        return theta_ == other.theta_ && phi_ == other.phi_
            && lambda_ == other.lambda_;
    }

private:
    double const theta_;
    double const phi_;
    double const lambda_;
};

} // namespace tweedledum::Op
