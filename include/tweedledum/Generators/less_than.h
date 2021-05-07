/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Wire.h"
#include "../Operators/Standard/X.h"

// Applies a less-than comparison between two integers encoded into qubits.  It
// flips a target qubit based on the result of the comparison.
//
// |0>|b>|a> = |(a < b)>|b>|a>
//
// The implementation is based on a ripple carry adder using the trick that
// a - b = (a' - b)' (where ' denotes the ones complement)
//
namespace tweedledum {
namespace deprecated {
// Implementation based on TTK ripple carry adder.  This basically just make
// a one complement by adding inverters (NOT) gates.
inline void less_than_ttk(
  Circuit& circuit, std::vector<Qubit> a, std::vector<Qubit> const& b, Qubit lt)
{
    assert(a.size() == b.size());
    uint32_t const n = a.size();
    for (Qubit w : a) {
        circuit.apply_operator(Op::X(), {w});
    }
    a.push_back(lt);
    // Step 1
    for (uint32_t i = 1; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
    // Step 2
    for (uint32_t i = n; i-- > 1;) {
        circuit.apply_operator(Op::X(), {a[i], a[i + 1]});
    }
    // Step 3
    for (uint32_t i = 0; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i], a[i + 1]});
    }
    // Step 4
    for (uint32_t i = n; i-- > 1;) {
        circuit.apply_operator(Op::X(), {a[i - 1], b[i - 1], a[i]});
    }
    // Step 5
    for (uint32_t i = 1; i < n - 1; ++i) {
        circuit.apply_operator(Op::X(), {a[i], a[i + 1]});
    }
    // Step 6
    for (uint32_t i = 1; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
    for (uint32_t i = 0; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i]});
    }
}
} // namespace deprecated

// This is a slightly better version.  The only difference here is that the
// inversters are absorbed into the the controls of the Toffoli gates.
inline void less_than_ttk(
  Circuit& circuit, std::vector<Qubit> a, std::vector<Qubit> const& b, Qubit lt)
{
    assert(a.size() == b.size());
    uint32_t const n = a.size();
    a.push_back(lt);
    // Step 1
    for (uint32_t i = 1; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
    // Step 2
    for (uint32_t i = n; i-- > 1;) {
        circuit.apply_operator(Op::X(), {a[i], a[i + 1]});
    }
    // Step 3
    circuit.apply_operator(Op::X(), {!a[0], b[0], a[1]});
    circuit.apply_operator(Op::X(), {!a[1], !b[1], a[2]});
    for (uint32_t i = 2; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], !b[i], a[i + 1]});
    }
    // Step 4
    for (uint32_t i = n; i-- > 3;) {
        circuit.apply_operator(Op::X(), {a[i - 1], !b[i - 1], a[i]});
    }
    circuit.apply_operator(Op::X(), {!a[1], !b[1], a[2]});
    circuit.apply_operator(Op::X(), {!a[0], b[0], a[1]});
    // Step 5
    for (uint32_t i = 1; i < n - 1; ++i) {
        circuit.apply_operator(Op::X(), {a[i], a[i + 1]});
    }
    // Step 6
    for (uint32_t i = 1; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
    circuit.apply_operator(Op::X(), {lt});
}

// Generic function that takes the one I think is best (:
inline void less_than(Circuit& circuit, std::vector<Qubit> const& a,
  std::vector<Qubit> const& b, Qubit carry)
{
    less_than_ttk(circuit, a, b, carry);
}

inline Circuit less_than(uint32_t n)
{
    Circuit circuit;
    std::vector<Qubit> a_qubits;
    std::vector<Qubit> b_qubits;
    for (uint32_t i = 0; i < n; ++i) {
        a_qubits.push_back(circuit.create_qubit(fmt::format("a{}", i)));
    }
    for (uint32_t i = 0; i < n; ++i) {
        b_qubits.push_back(circuit.create_qubit(fmt::format("b{}", i)));
    }
    Qubit carry = circuit.create_qubit();
    less_than(circuit, a_qubits, b_qubits, carry);
    return circuit;
}

} // namespace tweedledum
