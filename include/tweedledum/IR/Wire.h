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
    enum Kind : uint32_t { classical, quantum };

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
        return static_cast<Wire::Kind>(kind_);
    }

    enum Polarity : uint32_t { positive, negative };

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
        : uid_(uid), kind_(static_cast<uint32_t>(k)), polarity_(static_cast<uint32_t>(p))
    {}

    union {
        uint32_t data_;
        struct {
            uint32_t const uid_ : 30;
            uint32_t const kind_ : 1;
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
    WireStorage() = default;

    uint32_t num_wires() const
    {
        return num_qubits() + num_cbits();
    }

    uint32_t num_qubits() const
    {
        return qubits_.size();
    }

    uint32_t num_cbits() const
    {
        return cbits_.size();
    }

    WireRef qubit_ref(uint32_t const uid) const
    {
        assert(uid < num_qubits());
        return WireRef::qubit(uid);
    }

    std::vector<WireRef> qubits() const
    {
        std::vector<WireRef> qs;
        qs.reserve(num_qubits());
        for (Wire const& qubit : qubits_) {
            qs.push_back(WireRef::qubit(qubit.uid));
        }
        return qs;
    }

    template<typename Fn>
    void foreach_wire(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, WireRef> ||
                      std::is_invocable_r_v<void, Fn, Wire const&> ||
                      std::is_invocable_r_v<void, Fn, WireRef, Wire const&>);
        // clang-format on
        for (Wire const& qubit : qubits_) {
            if constexpr (std::is_invocable_r_v<void, Fn, WireRef>) {
                fn(WireRef::qubit(qubit.uid));
            } else if constexpr (std::is_invocable_r_v<void, Fn, Wire const&>) {
                fn(qubit);
            } else {
                fn(WireRef::qubit(qubit.uid), qubit);
            }
        }
        for (Wire const& cbit : cbits_) {
            if constexpr (std::is_invocable_r_v<void, Fn, WireRef>) {
                fn(WireRef::cbit(cbit.uid));
            } else if constexpr (std::is_invocable_r_v<void, Fn, Wire const&>) {
                fn(cbit);
            } else {
                fn(WireRef::cbit(cbit.uid), cbit);
            }
        }
    }

protected:
    WireRef do_create_qubit(std::string_view name)
    {
        uint32_t const uid = qubits_.size();
        qubits_.emplace_back(uid, name, Wire::Kind::quantum);
        return WireRef::qubit(uid);
    }

    WireRef do_create_cbit(std::string_view name)
    {
        uint32_t const uid = cbits_.size();
        cbits_.emplace_back(uid, name, Wire::Kind::classical);
        return WireRef::cbit(uid);
    }

private:
    std::vector<Wire> qubits_;
    std::vector<Wire> cbits_;
};

} // namespace tweedledum
