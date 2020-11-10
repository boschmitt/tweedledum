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

class WireStorage;

struct Wire {
    enum Kind { classical, quantum };

    uint32_t const uid;
    std::string const name;
    Kind const kind;

    Wire(uint32_t uid, std::string_view name, Kind kind)
        : uid(uid), name(name), kind(kind)
    {}

    // FIXME: maybe this it too much! :D
    operator uint32_t() const
    {
        return uid;
    }
};

class WireRef {
public:
    // // Copy constructor
    // WireRef(WireRef const& other)
    //     : data_(other.data_)
    // {}

    WireRef(WireRef const& other) = default;

    // Copy assignment
    WireRef& operator=(WireRef const& other)
    {
        data_ = other.data_;
        return *this;
    }

    // Return the sentinel value
    static constexpr WireRef invalid()
    {
        return WireRef();
    }

    uint32_t uid() const
    {
        return uid_;
    }

    Wire::Kind kind() const
    {
        return kind_;
    }

    enum Polarity { positive, negative };

    Polarity polarity() const
    {
        return static_cast<Polarity>(polarity_);
    }

    void complement()
    {
        polarity_ ^= 1u;
    }

    bool is_complemented() const
    {
        return polarity_;
    }

    WireRef operator!() const
    {
        WireRef complemented(*this);
        complemented.polarity_ ^= 1u;
        return complemented;
    }

    bool operator==(WireRef other) const
    {
        return data_ == other.data_;
    }

    bool operator!=(WireRef other) const
    {
        return data_ != other.data_;
    }

    operator uint32_t() const
    {
        return uid_;
    }

protected:
    friend class WireStorage;

    static constexpr WireRef qubit(uint32_t uid)
    {
        return WireRef(uid, Wire::Kind::quantum);
    }

    static constexpr WireRef cbit(uint32_t uid)
    {
        return WireRef(uid, Wire::Kind::classical);
    }

    constexpr WireRef(uint32_t uid, Wire::Kind k, Polarity p = Polarity::positive)
        : uid_(uid), kind_(k), polarity_(p)
    {}

    union {
        uint32_t data_;
        struct {
            uint32_t const uid_ : 30;
            Wire::Kind const kind_ : 1;
            uint32_t polarity_ : 1;
        };
    };

private:
    constexpr WireRef()
        : data_(std::numeric_limits<uint32_t>::max())
    {}
};

class WireStorage {
public:
    WireStorage() : num_qubits_(0u) {}

    uint32_t num_wires() const
    {
        return wires_.size();
    }

    uint32_t num_qubits() const
    {
        return num_qubits_;
    }

    uint32_t num_cbits() const
    {
        return num_wires() - num_qubits();
    }

    WireRef wire_ref(uint32_t idx) const
    {
        assert(idx < refs_.size());
        return refs_.at(idx);
    }

    auto begin_wire() const
    {
        return wires_.cbegin();
    }

    auto end_wire() const
    {
        return wires_.cend();
    }

    template<typename Fn>
    void foreach_wire(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, WireRef> ||
                      std::is_invocable_r_v<void, Fn, Wire const&> ||
                      std::is_invocable_r_v<void, Fn, WireRef, Wire const&>);
        // clang-format on
        for (uint32_t i = 0u; i < wires_.size(); ++i) {
            if constexpr (std::is_invocable_r_v<void, Fn, WireRef>) {
                fn(refs_.at(i));
            } else if constexpr (std::is_invocable_r_v<void, Fn, Wire const&>) {
                fn(wires_.at(i));
            } else {
                fn(refs_.at(i), wires_.at(i));
            }
        }
    }

protected:
    WireRef do_create_qubit(std::string_view name)
    {
        uint32_t uid = wires_.size();
        wires_.emplace_back(uid, name, Wire::Kind::quantum);
        num_qubits_++;
        refs_.push_back({uid, Wire::Kind::quantum});
        return refs_.back();
    }

    WireRef do_create_cbit(std::string_view name)
    {
        uint32_t uid = wires_.size();
        wires_.emplace_back(uid, name, Wire::Kind::classical);
        refs_.push_back({uid, Wire::Kind::classical});
        return refs_.back();
    }

private:
    uint32_t num_qubits_ = 0u;
    std::vector<WireRef> refs_;
    std::vector<Wire> wires_;
};

} // namespace tweedledum
