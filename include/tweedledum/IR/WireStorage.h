/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "Cbit.h"
#include "Qubit.h"

#include <cassert>
#include <limits>
#include <string>
#include <vector>

namespace tweedledum {

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

    Cbit cbit(uint32_t const uid) const
    {
        assert(uid < num_cbits());
        return Cbit(uid);
    }

    std::vector<Cbit> cbits() const
    {
        return cbits_;
    }

    Qubit qubit(uint32_t const uid) const
    {
        assert(uid < num_qubits());
        return Qubit(uid);
    }

    std::vector<Qubit> qubits() const
    {
        return qubits_;
    }

    std::string_view name(Cbit cbit) const
    {
        return cbit_names_.at(cbit);
    }

    std::string_view name(Qubit qubit) const
    {
        return qubit_names_.at(qubit);
    }

    template<typename Fn>
    void foreach_cbit(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, Cbit> ||
                      std::is_invocable_r_v<void, Fn, std::string_view> ||
                      std::is_invocable_r_v<void, Fn, Cbit, std::string_view>);
        // clang-format on
        if constexpr (std::is_invocable_r_v<void, Fn, Cbit>) {
            for (Cbit const& cbit : cbits_) {
                fn(cbit);
            }
        } else if constexpr (std::is_invocable_r_v<void, Fn, std::string_view>)
        {
            for (std::string const& name : cbit_names_) {
                fn(name);
            }
        } else {
            for (uint32_t i = 0; i < num_cbits(); ++i) {
                fn(cbits_.at(i), cbit_names_.at(i));
            }
        }
    }

    template<typename Fn>
    void foreach_qubit(Fn&& fn) const
    {
        // clang-format off
        static_assert(std::is_invocable_r_v<void, Fn, Qubit> ||
                      std::is_invocable_r_v<void, Fn, std::string_view> ||
                      std::is_invocable_r_v<void, Fn, Qubit, std::string_view>);
        // clang-format on
        if constexpr (std::is_invocable_r_v<void, Fn, Qubit>) {
            for (Qubit const& qubit : qubits_) {
                fn(qubit);
            }
        } else if constexpr (std::is_invocable_r_v<void, Fn, std::string_view>)
        {
            for (std::string const& name : qubit_names_) {
                fn(name);
            }
        } else {
            for (uint32_t i = 0; i < num_qubits(); ++i) {
                fn(qubits_.at(i), qubit_names_.at(i));
            }
        }
    }

protected:
    Cbit do_create_cbit(std::string_view name)
    {
        uint32_t const uid = cbits_.size();
        cbits_.push_back(Cbit(uid));
        cbit_names_.emplace_back(name);
        return cbits_.back();
    }

    Qubit do_create_qubit(std::string_view name)
    {
        uint32_t const uid = qubits_.size();
        qubits_.push_back(Qubit(uid));
        qubit_names_.emplace_back(name);
        return qubits_.back();
    }

private:
    std::vector<Cbit> cbits_;
    std::vector<std::string> cbit_names_;
    std::vector<Qubit> qubits_;
    std::vector<std::string> qubit_names_;
};

} // namespace tweedledum
