/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#pragma once

#include "../IR/Circuit.h"
#include "../IR/Wire.h"
#include "../Operators/Standard/X.h"

// The adders implemented here are based on the following papers:
//
// Cuccaro, Steven A., et al. "A new quantum ripple-carry addition circuit."
// arXiv preprint quant-ph/0410184 (2004).
//
// Takahashi, Yasuhiro, and Noboru Kunihiro. "A fast quantum circuit for
// addition with few qubits." Quantum Information & Computation 8.6 (2008):
// 636-649.
//
// Takahashi, Yasuhiro, Seiichiro Tani, and Noboru Kunihiro. "Quantum addition
// circuits and unbounded fan-out." arXiv preprint arXiv:0910.2530 (2009).
namespace tweedledum {
namespace deprecated {
// This is a literal translation of the algorithm given in Figure 5 of the
// Cuccaro et al. paper.
inline void carry_ripple_adder_inplace_cdkm(Circuit& circuit,
  std::vector<Qubit> const& a, std::vector<Qubit> const& b, Qubit carry)
{
    assert(a.size() == b.size());
    uint32_t const n = a.size();
    for (uint32_t i = 1; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
    Qubit x = circuit.request_ancilla();
    circuit.apply_operator(Op::X(), {a[1], x});
    circuit.apply_operator(Op::X(), {a[0], b[0], x});
    circuit.apply_operator(Op::X(), {a[2], a[1]});
    circuit.apply_operator(Op::X(), {x, b[1], a[1]});
    circuit.apply_operator(Op::X(), {a[3], a[2]});

    for (uint32_t i = 2; i < n - 2; ++i) {
        circuit.apply_operator(Op::X(), {a[i - 1], b[i], a[i]});
        circuit.apply_operator(Op::X(), {a[i + 2], a[i + 1]});
    }
    circuit.apply_operator(Op::X(), {a[n - 3], b[n - 2], a[n - 2]});
    circuit.apply_operator(Op::X(), {a[n - 1], carry});
    circuit.apply_operator(Op::X(), {a[n - 2], b[n - 1], carry});
    for (uint32_t i = 1; i < n - 1; ++i) {
        circuit.apply_operator(Op::X(), {b[i]});
    }

    circuit.apply_operator(Op::X(), {x, b[1]});
    for (uint32_t i = 2; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i - 1], b[i]});
    }

    circuit.apply_operator(Op::X(), {a[n - 3], b[n - 2], a[n - 2]});

    for (uint32_t i = n - 2; i-- > 2;) {
        circuit.apply_operator(Op::X(), {a[i - 1], b[i], a[i]});
        circuit.apply_operator(Op::X(), {a[i + 2], a[i + 1]});
        circuit.apply_operator(Op::X(), {b[i + 1]});
    }
    circuit.apply_operator(Op::X(), {x, b[1], a[1]});
    circuit.apply_operator(Op::X(), {a[3], a[2]});
    circuit.apply_operator(Op::X(), {b[2]});
    circuit.apply_operator(Op::X(), {a[0], b[0], x});
    circuit.apply_operator(Op::X(), {a[2], a[1]});
    circuit.apply_operator(Op::X(), {b[1]});
    circuit.apply_operator(Op::X(), {a[1], x});
    for (uint32_t i = 0; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
}
} // namespace deprecated

// This is a slightly better version of the algorithm given in Figure 5 of the
// Cuccaro et al. paper.  The only difference here is that the inversters are
// absorbed into the the controls of the Toffoli gates.
inline void carry_ripple_adder_inplace_cdkm(Circuit& circuit,
  std::vector<Qubit> const& a, std::vector<Qubit> const& b, Qubit carry)
{
    assert(a.size() == b.size());
    uint32_t const n = a.size();
    for (uint32_t i = 1; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
    Qubit x = circuit.request_ancilla();
    circuit.apply_operator(Op::X(), {a[1], x});
    circuit.apply_operator(Op::X(), {a[0], b[0], x});
    circuit.apply_operator(Op::X(), {a[2], a[1]});
    circuit.apply_operator(Op::X(), {x, b[1], a[1]});
    circuit.apply_operator(Op::X(), {a[3], a[2]});

    for (uint32_t i = 2; i < n - 2; ++i) {
        circuit.apply_operator(Op::X(), {a[i - 1], b[i], a[i]});
        circuit.apply_operator(Op::X(), {a[i + 2], a[i + 1]});
    }
    circuit.apply_operator(Op::X(), {a[n - 3], b[n - 2], a[n - 2]});
    circuit.apply_operator(Op::X(), {a[n - 1], carry});
    circuit.apply_operator(Op::X(), {a[n - 2], b[n - 1], carry});
    circuit.apply_operator(Op::X(), {x, b[1]});
    for (uint32_t i = 2; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i - 1], b[i]});
    }

    circuit.apply_operator(Op::X(), {a[n - 3], !b[n - 2], a[n - 2]});

    for (uint32_t i = n - 2; i-- > 2;) {
        circuit.apply_operator(Op::X(), {a[i - 1], !b[i], a[i]});
        circuit.apply_operator(Op::X(), {a[i + 2], a[i + 1]});
    }
    circuit.apply_operator(Op::X(), {x, !b[1], a[1]});
    circuit.apply_operator(Op::X(), {a[3], a[2]});
    circuit.apply_operator(Op::X(), {a[0], !b[0], x});
    circuit.apply_operator(Op::X(), {a[2], a[1]});
    circuit.apply_operator(Op::X(), {a[1], x});
    for (uint32_t i = 0; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
}

// This is an implementation based on Figure 4 of the Cuccaro et al. paper.
inline void carry_ripple_adder_inplace_cdkm_v1(Circuit& circuit,
  std::vector<Qubit> const& a, std::vector<Qubit> const& b, Qubit carry)
{
    assert(a.size() == b.size());
    uint32_t const n = a.size();
    Qubit x = circuit.request_ancilla();
    // MAJ(x, b0, a0)
    circuit.apply_operator(Op::X(), {a[0], b[0]});
    circuit.apply_operator(Op::X(), {a[0], x});
    circuit.apply_operator(Op::X(), {x, b[0], a[0]});
    // MAJ(a[i - 1], b[i], a[i])
    for (uint32_t i = 1; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
        circuit.apply_operator(Op::X(), {a[i], a[i - 1]});
        circuit.apply_operator(Op::X(), {a[i - 1], b[i], a[i]});
    }
    circuit.apply_operator(Op::X(), {a[n - 1], carry});
    // UMAJ(a[i - 1], b[i], a[i])
    for (uint32_t i = n; i-- > 1;) {
        circuit.apply_operator(Op::X(), {a[i - 1], b[i], a[i]});
        circuit.apply_operator(Op::X(), {a[i], a[i - 1]});
        circuit.apply_operator(Op::X(), {a[i - 1], b[i]});
    }
    // UMAJ(x, b0, a0)
    circuit.apply_operator(Op::X(), {x, b[0], a[0]});
    circuit.apply_operator(Op::X(), {a[0], x});
    circuit.apply_operator(Op::X(), {x, b[0]});
}

// Ripple-Carry approach with depth O(n)
inline void carry_ripple_adder_inplace_ttk(Circuit& circuit,
  std::vector<Qubit> a, std::vector<Qubit> const& b, Qubit carry)
{
    assert(a.size() == b.size());
    uint32_t const n = a.size();
    // Do this so the construction is the same as the paper
    a.push_back(carry);
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
        circuit.apply_operator(Op::X(), {a[i], b[i]});
        circuit.apply_operator(Op::X(), {a[i - 1], b[i - 1], a[i]});
    }
    // Step 5
    for (uint32_t i = 1; i < n - 1; ++i) {
        circuit.apply_operator(Op::X(), {a[i], a[i + 1]});
    }
    // Step 6
    for (uint32_t i = 0; i < n; ++i) {
        circuit.apply_operator(Op::X(), {a[i], b[i]});
    }
}

// Generic function that takes the adder I think is best (:
inline void carry_ripple_adder_inplace(Circuit& circuit,
  std::vector<Qubit> const& a, std::vector<Qubit> const& b, Qubit carry)
{
    carry_ripple_adder_inplace_ttk(circuit, a, b, carry);
}

inline Circuit carry_ripple_adder_inplace(uint32_t n)
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
    carry_ripple_adder_inplace_ttk(circuit, a_qubits, b_qubits, carry);
    return circuit;
}

} // namespace tweedledum
