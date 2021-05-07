/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/Matrix.h"

#include <string_view>

namespace tweedledum::Op {

class P {
public:
    static constexpr std::string_view kind()
    {
        return "std.p";
    }

    P(double angle)
        : angle_(angle)
    {}

    P adjoint() const
    {
        return P(-angle_);
    }

    double angle() const
    {
        return angle_;
    }

    UMatrix2 const matrix() const
    {
        Complex const a = std::exp(Complex(0., angle_));
        // clang-format off
        return (UMatrix2() << 1., 0.,
                              0., a).finished();
        // clang-format on
    }

    bool operator==(P const& other) const
    {
        return angle_ == other.angle_;
    }

private:
    double const angle_;
};

} // namespace tweedledum::Op
