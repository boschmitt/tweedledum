/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Instruction.h"
#include "Wire.h"
#include "../Utils/Angle.h"

#include <cassert>
#include <fmt/format.h>
#include <memory>
#include <vector>
#include <utility>

namespace tweedledum {

class BaseView;

class Circuit : public WireStorage {
public:
    Circuit() : global_phase_(sym_angle::zero)
    {
        instructions_.reserve(1024);
    }

    // Properties
    Angle& global_phase()
    {
        return global_phase_;
    }

    Angle global_phase() const
    {
        return global_phase_;
    }

    uint32_t size() const
    {
        return instructions_.size();
    }

    uint32_t num_ancillae() const
    {
        return free_ancillae_.size();
    }

    // Wires
    WireRef create_qubit(std::string_view name)
    {
        last_instruction_.emplace_back(InstRef::invalid());
        return do_create_qubit(name);
    }

    WireRef create_qubit()
    {
        std::string const name = fmt::format("__q{}", num_qubits());
        return create_qubit(name);
    }

    void create_ancilla()
    {
        WireRef const qubit = create_qubit(fmt::format("__a{}", num_qubits()));
        free_ancillae_.push_back(qubit);
    }

    WireRef request_ancilla()
    {
        if (free_ancillae_.empty()) {
            return create_qubit(fmt::format("__a{}", num_qubits()));
        } else {
            WireRef qubit = free_ancillae_.back();
            free_ancillae_.pop_back();
            return qubit;
        }
    }

    void release_ancilla(WireRef qubit)
    {
        free_ancillae_.push_back(qubit);
    }

    WireRef create_cbit(std::string_view name)
    {
        last_instruction_.emplace_back(InstRef::invalid());
        return do_create_cbit(name);
    }

    WireRef create_cbit()
    {
        std::string const name = fmt::format("__c{}", num_cbits());
        return create_cbit(name);
    }

    // Instructions
    template<typename OpT>
    InstRef apply_operator(OpT&& optor, std::vector<WireRef> const& wires)
    {
        Instruction& inst = instructions_.emplace_back(std::forward<OpT>(optor), wires);
        connect_instruction(inst);
        return InstRef(instructions_.size() - 1);
    }

    // template<typename OpT>
    // InstRef apply_operator(OpT const& optor, std::vector<WireRef> const& wires)
    // {
    //     Instruction& inst = instructions_.emplace_back(optor, wires);
    //     connect_instruction(inst);
    //     return InstRef(instructions_.size() - 1);
    // }

    InstRef apply_operator(Instruction const& optor, std::vector<WireRef> const& wires)
    {
        Instruction& inst = instructions_.emplace_back(optor, wires);
        connect_instruction(inst);
        return InstRef(instructions_.size() - 1);
    }

    // FIXME: maybe remove this, if so need to change the python bindings!
    InstRef apply_operator(Instruction const& optor)
    {
        Instruction& inst = instructions_.emplace_back(optor);
        connect_instruction(inst);
        return InstRef(instructions_.size() - 1);
    }

    // Composition
    void append(Circuit const& other, std::vector<WireRef> const& wires)
    {
        assert(other.num_wires() == wires.size());
        other.foreach_instruction([&](Instruction const& inst) {
            std::vector<WireRef> this_wires;
            inst.foreach_wire([&](WireRef wire) {
                this_wires.push_back(wires.at(wire));
            });
            assert(!this_wires.empty());
            apply_operator(inst, this_wires);
        });
    }

    Instruction const& instruction(InstRef ref) const
    {
        return instructions_.at(ref);
    }

    // Output: the last instruction of a qubit
    // FIXME: this is a hack for now, need to think about it a bit more
    template<typename Fn>
    void foreach_output(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, InstRef> ||
                      std::is_invocable_r_v<void, Fn, Instruction const&> ||
                      std::is_invocable_r_v<void, Fn, InstRef, Instruction const&>);
        // clang-format on
        for (InstRef ref : last_instruction_) {
            if (ref == InstRef::invalid()) {
                continue;
            }
            if constexpr (std::is_invocable_r_v<void, Fn, InstRef>) {
                fn(ref);
            } else if constexpr (std::is_invocable_r_v<void, Fn, Instruction const&>) {
                fn(instructions_.at(ref));
            } else {
                fn(ref, instructions_.at(ref));
            }
        }
    }

    template<typename Fn>
    void foreach_instruction(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, InstRef> ||
                      std::is_invocable_r_v<void, Fn, Instruction const&> ||
                      std::is_invocable_r_v<void, Fn, InstRef, Instruction const&>);
        // clang-format on
        for (uint32_t i = 0u; i < instructions_.size(); ++i) {
            if constexpr (std::is_invocable_r_v<void, Fn, InstRef>) {
                fn(InstRef(i));
            } else if constexpr (std::is_invocable_r_v<void, Fn, Instruction const&>) {
                fn(instructions_.at(i));
            } else {
                fn(InstRef(i), instructions_.at(i));
            }
        }
    }

    template<typename Fn>
    void foreach_r_instruction(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, InstRef> ||
                      std::is_invocable_r_v<void, Fn, Instruction const&> ||
                      std::is_invocable_r_v<void, Fn, InstRef, Instruction const&>);
        // clang-format on
        for (uint32_t i = instructions_.size(); i --> 0u;) {
            if constexpr (std::is_invocable_r_v<void, Fn, InstRef>) {
                fn(InstRef(i));
            } else if constexpr (std::is_invocable_r_v<void, Fn, Instruction const&>) {
                fn(instructions_.at(i));
            } else {
                fn(InstRef(i), instructions_.at(i));
            }
        }
    }

    template<typename Fn>
    void foreach_child(InstRef ref, Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, InstRef> ||
                      std::is_invocable_r_v<void, Fn, Instruction const&> ||
                      std::is_invocable_r_v<void, Fn, InstRef, Instruction const&>);
        // clang-format on
        Instruction const& inst = instructions_.at(ref);
        inst.foreach_wire([&](InstRef const iref) {
            if constexpr (std::is_invocable_r_v<void, Fn, InstRef>) {
                fn(iref);
            } else if constexpr (std::is_invocable_r_v<void, Fn, Instruction const&>) {
                fn(instructions_.at(iref));
            } else {
                fn(iref, instructions_.at(iref));
            }
        });
    }

    // This methods are needed for the python bindings 
    auto py_begin() const
    {
        return instructions_.begin();
    }

    auto py_end() const
    {
        return instructions_.end();
    }

private:
    friend class BaseView;

    void connect_instruction(Instruction& inst)
    {
        for (auto& [wref, iref] : inst.qubits_) {
            iref = last_instruction_.at(wref);
            last_instruction_.at(wref).uid_ = instructions_.size() - 1;
        }
        for (auto& [wref, iref] : inst.cbits_) {
            iref = last_instruction_.at(wref);
            last_instruction_.at(wref).uid_ = instructions_.size() - 1;
        }
    }

    std::vector<Instruction, Instruction::Allocator> instructions_;
    std::vector<InstRef> last_instruction_; // last instruction on a wire
    std::vector<WireRef> free_ancillae_; // Should this be here?!
    Angle global_phase_;
};

} // namespace tweedledum
