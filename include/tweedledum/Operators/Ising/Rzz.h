/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/Matrix.h"

#include <cmath>
#include <string_view>

namespace tweedledum::Op {

class Rzz {
public:
    static constexpr std::string_view kind()
    {
        return "ising.rzz";
    }

    Rzz(double angle)
        : angle_(angle)
    {}

    Rzz adjoint() const
    {
        return Rzz(-angle_);
    }

    UMatrix4 const matrix() const
    {
        Complex const p = std::exp(Complex(0., angle_ / 2));
        Complex const n = std::exp(Complex(0., -angle_ / 2));
        // clang-format off
        return (UMatrix4() << p, 0, 0, 0,
                              0, n, 0, 0,
                              0, 0, n, 0,
                              0, 0, 0, p).finished();
        // clang-format on
    }

    uint32_t num_targets() const
    {
        return 2u;
    }

    bool operator==(Rzz const& other) const
    {
        return angle_ == other.angle_;
    }

private:
    double const angle_;
};

} // namespace tweedledum::Op
