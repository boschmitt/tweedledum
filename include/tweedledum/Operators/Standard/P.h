/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/Angle.h"
#include "../../Utils/Matrix.h"

#include <string_view>

namespace tweedledum::Op {

class P {
public:
    static constexpr std::string_view kind()
    {
        return "std.p";
    }

    P(Angle angle) : angle_(angle)
    {}

    P adjoint() const
    {
        return P(-angle_);
    }

    Angle angle() const
    {
        return angle_;
    }

    UMatrix2 const matrix() const
    {
        Complex const a = std::exp(Complex(0., angle_.numeric_value()));
        return (UMatrix2() << 1., 0.,
                              0., a).finished();
    }

    bool operator==(P const& other) const
    {
        return angle_ == other.angle_;
    }

private:
    Angle const angle_;
};

} // namespace tweedledum
