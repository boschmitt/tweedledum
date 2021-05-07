/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

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

    Rz(double angle)
        : angle_(angle)
    {}

    Rz adjoint() const
    {
        return Rz(-angle_);
    }

    double angle() const
    {
        return angle_;
    }

    UMatrix2 const matrix() const
    {
        return (UMatrix2() << std::exp(Complex(0., -angle_ / 2.)), 0., 0,
          std::exp(Complex(0., angle_ / 2.)))
          .finished();
    }

    bool operator==(Rz const& other) const
    {
        return angle_ == other.angle_;
    }

private:
    double const angle_;
};

} // namespace tweedledum::Op
