/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../Utils/SmallVector.h"
#include "Operator.h"
#include "Wire.h"

#include <cassert>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

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
        : Operator(static_cast<Operator const&>(other))
        , qubits_conns_(other.qubits_conns_)
        , cbits_conns_(other.cbits_conns_)
    {}

    Instruction(Instruction const& other, std::vector<Qubit> const& qubits,
      std::vector<Cbit> const& cbits)
        : Operator(static_cast<Operator const&>(other))
    {
        for (Qubit qubit : qubits) {
            qubits_conns_.emplace_back(qubit, InstRef::invalid());
        }
        for (Cbit cbit : cbits) {
            cbits_conns_.emplace_back(cbit, InstRef::invalid());
        }
        assert(qubits_conns_.size() >= this->num_targets());
    }

    uint32_t num_controls() const
    {
        return qubits_conns_.size() - this->num_targets();
    }

    Qubit control(uint32_t const idx = 0u) const
    {
        assert(idx < num_controls());
        return qubits_conns_[idx].qubit;
    }

    Qubit target(uint32_t const idx = 0u) const
    {
        assert(idx < this->num_targets());
        return qubits_conns_[num_controls() + idx].qubit;
    }

    uint32_t num_qubits() const
    {
        return qubits_conns_.size();
    }

    uint32_t num_cbits() const
    {
        return cbits_conns_.size();
    }

    uint32_t num_wires() const
    {
        return num_qubits() + num_cbits();
    }

    Qubit cbit(uint32_t idx) const
    {
        assert(idx < qubits_conns_.size());
        return qubits_conns_[idx].qubit;
    }

    std::vector<Cbit> cbits() const
    {
        std::vector<Cbit> cbits;
        cbits.reserve(cbits_conns_.size());
        std::transform(cbits_conns_.begin(), cbits_conns_.end(),
          std::back_inserter(cbits),
          [](CbitConnection const& c) -> Cbit { return c.cbit; });
        return cbits;
    }

    Qubit qubit(uint32_t idx) const
    {
        assert(idx < qubits_conns_.size());
        return qubits_conns_[idx].qubit;
    }

    std::vector<Qubit> qubits() const
    {
        std::vector<Qubit> qubits;
        qubits.reserve(qubits_conns_.size());
        std::transform(qubits_conns_.begin(), qubits_conns_.end(),
          std::back_inserter(qubits),
          [](QubitConnection const& c) -> Qubit { return c.qubit; });
        return qubits;
    }

    bool is_adjoint(Instruction const& other) const
    {
        if (qubits_conns_ != other.qubits_conns_) {
            return false;
        }
        if (cbits_conns_ != other.cbits_conns_) {
            return false;
        }
        std::optional<Operator> adj = other.adjoint();
        if (!adj) {
            return false;
        }
        return static_cast<Operator const&>(*this) == *adj;
    }

    template<typename Fn>
    void foreach_cbit(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, Cbit, InstRef> ||
                      std::is_invocable_r_v<void, Fn, Cbit> ||
                      std::is_invocable_r_v<void, Fn, InstRef>);
        // clang-format on
        for (CbitConnection const& connection : cbits_conns_) {
            if constexpr (std::is_invocable_r_v<void, Fn, Cbit, InstRef>) {
                fn(connection.cbit, connection.inst_ref);
            } else if constexpr (std::is_invocable_r_v<void, Fn, Cbit>) {
                fn(connection.cbit);
            } else {
                if (connection.inst_ref == InstRef::invalid()) {
                    continue;
                }
                fn(connection.inst_ref);
            }
        }
    }

    template<typename Fn>
    void foreach_qubit(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, Qubit, InstRef> ||
                      std::is_invocable_r_v<void, Fn, Qubit> ||
                      std::is_invocable_r_v<void, Fn, InstRef>);
        // clang-format on
        for (QubitConnection const& connection : qubits_conns_) {
            if constexpr (std::is_invocable_r_v<void, Fn, Qubit, InstRef>) {
                fn(connection.qubit, connection.inst_ref);
            } else if constexpr (std::is_invocable_r_v<void, Fn, Qubit>) {
                fn(connection.qubit);
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
        static_assert(std::is_invocable_r_v<void, Fn, Qubit>);
        for (uint32_t i = 0; i < qubits_conns_.size() - num_targets(); ++i) {
            fn(qubits_conns_[i].qubit);
        }
    }

    template<typename Fn>
    void foreach_target(Fn&& fn) const
    {
        static_assert(std::is_invocable_r_v<void, Fn, Qubit>);
        for (uint32_t i = num_controls(); i < qubits_conns_.size(); ++i) {
            fn(qubits_conns_[i].qubit);
        }
    }

    bool operator==(Instruction const& other) const
    {
        if (qubits_conns_ != other.qubits_conns_) {
            return false;
        }
        if (cbits_conns_ != other.cbits_conns_) {
            return false;
        }
        return static_cast<Operator const&>(*this)
            == static_cast<Operator const&>(other);
    }

private:
    struct QubitConnection {
        Qubit qubit;
        InstRef inst_ref;
        QubitConnection(Qubit qubit, InstRef i)
            : qubit(qubit)
            , inst_ref(i)
        {}

        // FIXME: this quite counterintuitive
        bool operator==(QubitConnection const& other) const
        {
            return qubit == other.qubit;
        }
    };

    struct CbitConnection {
        Cbit cbit;
        InstRef inst_ref;
        CbitConnection(Cbit cbit, InstRef i)
            : cbit(cbit)
            , inst_ref(i)
        {}

        // FIXME: this quite counterintuitive
        bool operator==(CbitConnection const& other) const
        {
            return cbit == other.cbit;
        }
    };

    friend class Circuit;

    template<typename OpT>
    Instruction(OpT&& optor, std::vector<Qubit> const& qubits,
      std::vector<Cbit> const& cbits)
        : Operator(std::forward<OpT>(optor))
    {
        for (Qubit qubit : qubits) {
            qubits_conns_.emplace_back(qubit, InstRef::invalid());
        }
        for (Cbit cbit : cbits) {
            cbits_conns_.emplace_back(cbit, InstRef::invalid());
        }
        assert(qubits_conns_.size() >= this->num_targets());
    }

    // TODO: Come up with a good value for the size of the inline buffer!
    SmallVector<QubitConnection, 3> qubits_conns_;
    SmallVector<CbitConnection, 1> cbits_conns_;

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
