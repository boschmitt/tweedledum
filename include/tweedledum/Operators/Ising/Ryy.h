/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../../Utils/Matrix.h"

#include <cmath>
#include <string_view>

namespace tweedledum::Op {

class Ryy {
public:
    static constexpr std::string_view kind()
    {
        return "ising.ryy";
    }

    Ryy(double angle)
        : angle_(angle)
    {}

    Ryy adjoint() const
    {
        return Ryy(-angle_);
    }

    UMatrix4 const matrix() const
    {
        Complex const a = std::cos(angle_);
        Complex const b{0., std::sin(angle_)};

        // clang-format off
        return (UMatrix4() << a,  0,  0, b,
                              0,  a, -b, 0,
                              0, -b,  a, 0,
                              b,  0,  0, a).finished();
        // clang-format on
    }

    uint32_t num_targets() const
    {
        return 2u;
    }

    bool operator==(Ryy const& other) const
    {
        return angle_ == other.angle_;
    }

private:
    double const angle_;
};

} // namespace tweedledum::Op
