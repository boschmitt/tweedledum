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

class Ry {
public:
    static constexpr std::string_view kind()
    {
        return "std.ry";
    }

    Ry(Angle angle) : angle_(angle)
    {}

    Ry adjoint() const
    {
        return Ry(-angle_);
    }

    UMatrix2 const matrix() const
    {
        return (UMatrix2() << std::cos(angle_.numeric_value() / 2.),
                              -std::sin(angle_.numeric_value() / 2.),
                              std::sin(angle_.numeric_value() / 2.),
                              std::cos(angle_.numeric_value() / 2.)).finished();
    }

    bool operator==(Ry const& other) const
    {
        return angle_ == other.angle_;
    }

private:
    Angle const angle_;
};

} // namespace tweedledum
