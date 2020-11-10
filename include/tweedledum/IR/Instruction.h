/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Operator.h"
#include "Wire.h"
#include "../Utils/SmallVector.h"

#include <cassert>
#include <limits>
#include <memory>
#include <vector>
#include <utility>

namespace tweedledum {

class Circuit;

struct InstRef {
    static constexpr InstRef invalid()
    {
        return InstRef(std::numeric_limits<uint32_t>::max());
    }

    constexpr explicit InstRef(uint32_t id)
        : uid_(id)
    {}

    uint32_t uid() const
    {
        return uid_;
    }

    bool operator==(InstRef other) const
    {
        return uid_ == other.uid_;
    }

    operator uint32_t() const
    {
        return uid_;
    }

private:
    friend class Circuit;

    uint32_t uid_;
};

class Instruction : public Operator {
public:
    // This is called by realloc!!
    Instruction(Instruction const& other)
        : Operator(static_cast<Operator const&>(other)), qubits_(other.qubits_)
        , cbits_(other.cbits_)
    {}

    Instruction(Instruction const& other, std::vector<WireRef> const& wires)
        : Operator(static_cast<Operator const&>(other))
    {
        for (WireRef ref : wires) {
            if (ref.kind() == Wire::Kind::quantum) {
                qubits_.emplace_back(ref, InstRef::invalid());
            } else {
                cbits_.emplace_back(ref, InstRef::invalid());
            }
        }
    }

    uint32_t num_controls() const
    {
        return qubits_.size() - this->num_targets();
    }

    WireRef control(uint32_t const idx = 0u) const 
    {
        assert(idx < num_controls());
        return qubits_[idx].wire_ref;
    }

    WireRef target(uint32_t const idx = 0u) const 
    {
        assert(idx < this->num_targets());
        return qubits_[num_controls() + idx].wire_ref;
    }

    uint32_t num_qubits() const
    {
        return qubits_.size();
    }

    uint32_t num_cbits() const
    {
        return cbits_.size();
    }

    uint32_t num_wires() const
    {
        return num_qubits() + num_cbits();
    }
 
    WireRef qubit(uint32_t idx) const
    {
        assert(idx < qubits_.size());
        return qubits_[idx].wire_ref;
    }

    std::vector<WireRef> qubits() const
    {
        std::vector<WireRef> qubits;
        qubits.reserve(qubits_.size());
        std::transform(qubits_.begin(), qubits_.end(), std::back_inserter(qubits),
        [](Connection const& c) -> WireRef { return c.wire_ref; });
        return qubits;
    }

    std::vector<WireRef> wires() const
    {
        std::vector<WireRef> wires;
        wires.reserve(qubits_.size()+ cbits_.size());
        std::transform(qubits_.begin(), qubits_.end(), std::back_inserter(wires),
        [](Connection const& c) -> WireRef { return c.wire_ref; });
        std::transform(cbits_.begin(), cbits_.end(), std::back_inserter(wires),
        [](Connection const& c) -> WireRef { return c.wire_ref; });
        return wires;
    }

    bool is_adjoint(Instruction const& other) const
    {
        if (qubits_ != other.qubits_) {
            return false;
        }
        if (cbits_ != other.cbits_) {
            return false;
        }
        std::optional<Operator> adj = other.adjoint();
        if (!adj) {
            return false;
        }
        return static_cast<Operator const&>(*this) == *adj;
    }

    // bool is_dependent(Instruction const& other) const
    // {
    //     // Need to check the Operators
    //     return true;
    // }
    
    template<typename Fn>
    void foreach_wire(Fn&& fn) const
    {
        static_assert(std::is_invocable_r_v<void, Fn, WireRef> ||
                      std::is_invocable_r_v<void, Fn, InstRef>);

        for (Connection const& connection : qubits_) {
            if constexpr (std::is_invocable_r_v<void, Fn, WireRef>) {
                fn(connection.wire_ref);
            } else {
                if (connection.inst_ref == InstRef::invalid()) {
                    continue;
                }
                fn(connection.inst_ref);
            }
        }
        for (Connection const& connection : cbits_) {
            if constexpr (std::is_invocable_r_v<void, Fn, WireRef>) {
                fn(connection.wire_ref);
            } else {
                if (connection.inst_ref == InstRef::invalid()) {
                    continue;
                }
                fn(connection.inst_ref);
            }
        }
    }

    template<typename Fn>
    void foreach_control(Fn&& fn) const
    {
        static_assert(std::is_invocable_r_v<void, Fn, WireRef>);
        for (uint32_t i = 0; i < qubits_.size() - num_targets(); ++i) {
            fn(qubits_[i].wire_ref);
        }
    }

    template<typename Fn>
    void foreach_target(Fn&& fn) const
    {
        static_assert(std::is_invocable_r_v<void, Fn, WireRef>);
        for (uint32_t i = num_controls(); i < qubits_.size(); ++i) {
            fn(qubits_[i].wire_ref);
        }
    }

    // FIXME: I want to make these at least protected..
    auto py_begin() const
    {
        return qubits_.begin();
    }

    auto py_end() const
    {
        return qubits_.end();
    }

    bool operator==(Instruction const& other) const
    {
        if (qubits_ != other.qubits_) {
            return false;
        }
        if (cbits_ != other.cbits_) {
            return false;
        }
        return static_cast<Operator const&>(*this) == static_cast<Operator const&>(other);
    }

private:
    struct Connection {
        WireRef wire_ref;
        InstRef inst_ref;
        Connection(WireRef w, InstRef i) : wire_ref(w), inst_ref(i)
        {}

        // FIXME: this quite counterintuitive
        bool operator==(Connection const& other) const
        {
            return wire_ref == other.wire_ref;
        }
    };

    friend class Circuit;

    template<typename OpT>
    Instruction(OpT&& optor, std::vector<WireRef> const& wires)
        : Operator(std::forward<OpT>(optor))
    {
        for (WireRef ref : wires) {
            if (ref.kind() == Wire::Kind::quantum) {
                qubits_.emplace_back(ref, InstRef::invalid());
            } else {
                cbits_.emplace_back(ref, InstRef::invalid());
            }
        }
    }

    // TODO: Come up with a good value for the size of the inline buffer!
    SmallVector<Connection, 3> qubits_;
    SmallVector<Connection, 1> cbits_;

    // I not sure about this:
    // This is needed because the constructor is private and a container such
    // as std::vector need to be able to create this
    struct Allocator : std::allocator<Instruction> {
        template<class U, class... Args>
        void construct(U* p, Args&&... args)
        {
            ::new ((void*) p) U(std::forward<Args>(args)...);
        }

        template<class U>
        struct rebind {
            typedef Allocator other;
        };
    };
    friend struct Allocator;
};

} // namespace tweedledum
