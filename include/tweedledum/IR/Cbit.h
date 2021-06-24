/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include <cassert>
#include <limits>
#include <string>
#include <vector>

namespace tweedledum {

class Cbit {
public:
    enum Polarity : uint32_t
    {
        positive = 0u,
        negative = 1u
    };

    // Return the sentinel value
    static constexpr Cbit invalid()
    {
        return Cbit();
    }

    constexpr Cbit(uint32_t uid, Polarity polarity = Polarity::positive)
        : uid_(uid)
        , polarity_(static_cast<uint32_t>(polarity))
    {}

    Cbit(Cbit const& other) = default;

    Cbit& operator=(Cbit const& other)
    {
        data_ = other.data_;
        return *this;
    }

    uint32_t uid() const
    {
        return uid_;
    }

    Polarity polarity() const
    {
        return static_cast<Polarity>(polarity_);
    }

    Cbit operator!() const
    {
        Cbit complemented(*this);
        complemented.polarity_ ^= 1u;
        return complemented;
    }

    Cbit operator+() const
    {
        return Cbit(uid_, Polarity::positive);
    }

    Cbit operator-() const
    {
        return Cbit(uid_, Polarity::negative);
    }

    bool operator==(Cbit other) const
    {
        return data_ == other.data_;
    }

    bool operator!=(Cbit other) const
    {
        return data_ != other.data_;
    }

    operator uint32_t() const
    {
        return uid_;
    }

protected:
    union {
        uint32_t data_;
        struct {
            uint32_t uid_ : 31;
            uint32_t polarity_ : 1;
        };
    };

private:
    constexpr Cbit()
        : data_(std::numeric_limits<uint32_t>::max())
    {}
};

} // namespace tweedledum
