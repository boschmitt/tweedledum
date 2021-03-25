/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Cbit.h"
#include "Instruction.h"
#include "Qubit.h"
#include "Wire.h"

#include <cassert>
#include <fmt/format.h>
#include <memory>
#include <vector>
#include <utility>

namespace tweedledum {

class BaseView;

class Circuit : public WireStorage {
public:
    Circuit() : global_phase_(0.0)
    {
        instructions_.reserve(1024);
    }

    // Properties
    double& global_phase()
    {
        return global_phase_;
    }

    double global_phase() const
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
    Qubit create_qubit(std::string_view name)
    {
        last_instruction_.emplace(last_instruction_.begin() + num_qubits(), 
                                  InstRef::invalid());
        return do_create_qubit(name);
    }

    Qubit create_qubit()
    {
        std::string const name = fmt::format("__q{}", num_qubits());
        return create_qubit(name);
    }

    void create_ancilla()
    {
        Qubit const qubit = create_qubit(fmt::format("__a{}", num_qubits()));
        free_ancillae_.push_back(qubit);
    }

    Qubit request_ancilla()
    {
        if (free_ancillae_.empty()) {
            return create_qubit(fmt::format("__a{}", num_qubits()));
        } else {
            Qubit qubit = free_ancillae_.back();
            free_ancillae_.pop_back();
            return qubit;
        }
    }

    void release_ancilla(Qubit qubit)
    {
        free_ancillae_.push_back(qubit);
    }

    Cbit create_cbit(std::string_view name)
    {
        last_instruction_.emplace_back(InstRef::invalid());
        return do_create_cbit(name);
    }

    Cbit create_cbit()
    {
        std::string const name = fmt::format("__c{}", num_cbits());
        return create_cbit(name);
    }

    // Instructions
    template<typename OpT>
    InstRef apply_operator(OpT&& optor, std::vector<Qubit> const& qubits,
        std::vector<Cbit> const& cbits = {})
    {
        Instruction& inst = instructions_.emplace_back(std::forward<OpT>(optor), qubits, cbits);
        connect_instruction(inst);
        return InstRef(instructions_.size() - 1);
    }

    InstRef apply_operator(Instruction const& optor, 
        std::vector<Qubit> const& qubits, std::vector<Cbit> const& cbits = {})
    {
        Instruction& inst = instructions_.emplace_back(optor, qubits, cbits);
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
    void append(Circuit const& other, std::vector<Qubit> const& qubits, 
        std::vector<Cbit> const& cbits)
    {
        assert(other.num_cbits() == cbits.size());
        assert(other.num_qubits() == qubits.size());

        other.foreach_instruction([&](Instruction const& inst) {
            std::vector<Qubit> this_qubits;
            inst.foreach_qubit([&](Qubit qubit) {
                Qubit const new_qubit = qubit.polarity() == Qubit::Polarity::positive ? qubits.at(qubit) : !qubits.at(qubit);
                this_qubits.push_back(new_qubit);
            });
            std::vector<Cbit> this_cbits;
            inst.foreach_cbit([&](Cbit cbit) {
                this_cbits.push_back(cbits.at(cbit));
            });
            assert(!this_qubits.empty());
            apply_operator(inst, this_qubits, this_cbits);
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
        inst.foreach_cbit([&](InstRef const iref) {
            if constexpr (std::is_invocable_r_v<void, Fn, InstRef>) {
                fn(iref);
            } else if constexpr (std::is_invocable_r_v<void, Fn, Instruction const&>) {
                fn(instructions_.at(iref));
            } else {
                fn(iref, instructions_.at(iref));
            }
        });
        inst.foreach_qubit([&](InstRef const iref) {
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
        uint32_t const inst_uid = instructions_.size() - 1;
        for (auto& [wref, iref] : inst.qubits_conns_) {
            iref = last_instruction_.at(wref);
            last_instruction_.at(wref).uid_ = inst_uid;
        }
        for (auto& [wref, iref] : inst.cbits_conns_) {
            iref = last_instruction_.at(wref);
            last_instruction_.at(wref + num_qubits()).uid_ = inst_uid;
        }
    }

    std::vector<Instruction, Instruction::Allocator> instructions_;
    std::vector<InstRef> last_instruction_; // last instruction on a wire
    std::vector<Qubit> free_ancillae_; // Should this be here?!
    double global_phase_;
};

} // namespace tweedledum
