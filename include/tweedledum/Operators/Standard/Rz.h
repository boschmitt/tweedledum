/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/Angle.h"
#include "../../Utils/Matrix.h"

#include <cmath>
#include <string_view>

namespace tweedledum::Op {

class Rz {
public:
    static constexpr std::string_view kind()
    {
        return "std.rz";
    }

    Rz(Angle angle) : angle_(angle)
    {}

    Rz adjoint() const
    {
        return Rz(-angle_);
    }

    static Angle angle() 
    {
        return sym_angle::pi;
    }

    UMatrix2 const matrix() const
    {
        return (UMatrix2() << std::exp(Complex(0., -angle_.numeric_value() / 2)), 0.,
        0, std::exp(Complex(0., angle_.numeric_value() / 2))).finished();
    }

    bool operator==(Rz const& other) const
    {
        return angle_ == other.angle_;
    }

private:
    Angle const angle_;
};

} // namespace tweedledum
