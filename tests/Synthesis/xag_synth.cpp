/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
// FIXME: There are conflicts among SAT solvers, so this header need to appear
//        first! Quite weird!
#include "tweedledum/Synthesis/xag_synth.h"
#include <mockturtle/algorithms/equivalence_checking.hpp>

#include "tweedledum/IR/Circuit.h"
#include "tweedledum/IR/Wire.h"
#include "tweedledum/Operators/Extension/Parity.h"

#include <catch.hpp>
#include <mockturtle/algorithms/miter.hpp>
#include <mockturtle/algorithms/xag_optimization.hpp>
#include <mockturtle/generators/arithmetic.hpp>
#include <mockturtle/generators/control.hpp>
#include <mockturtle/generators/modular_arithmetic.hpp>
#include <mockturtle/networks/xag.hpp>
#include <vector>

// Helper function
namespace tweedledum {

mockturtle::xag_network to_xag_network(
  Circuit const& circuit, uint32_t num_i, uint32_t num_o)
{
    using Signal = typename mockturtle::xag_network::signal;

    auto network = mockturtle::xag_network();
    std::vector<Signal> to_signal(
      circuit.num_qubits(), network.get_constant(false));
    for (uint32_t i = 0; i < num_i; ++i) {
        to_signal[i] = network.create_pi();
    }
    circuit.foreach_instruction([&](Instruction const& inst) {
        std::vector<Signal> signals;
        inst.foreach_qubit([&](Qubit w) {
            signals.push_back(
              to_signal[w.uid()] ^ (w.polarity() == Qubit::Polarity::negative));
        });
        if (inst.is_a<Op::Parity>()) {
            to_signal[inst.target().uid()] = network.create_nary_xor(signals);
            return;
        }
        Signal const ctrl =
          network.create_nary_and({signals.begin(), signals.end() - 1});
        to_signal[inst.target().uid()] =
          network.create_xor(signals.back(), ctrl);
    });
    for (uint32_t i = 0; i < num_o; ++i) {
        network.create_po(to_signal[num_i + i]);
    }
    return network;
}
} // namespace tweedledum

TEST_CASE("Synthesize constant gate", "[xag][synth]")
{
    using namespace tweedledum;
    SECTION("No inputs, one constant output")
    {
        auto oracle = mockturtle::xag_network();
        oracle.create_po(oracle.get_constant(false));

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("No inputs, two constant outputs")
    {
        auto oracle = mockturtle::xag_network();
        oracle.create_po(oracle.get_constant(false));
        oracle.create_po(oracle.get_constant(false));

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("No inputs, one nagated constant output")
    {
        auto oracle = mockturtle::xag_network();
        oracle.create_po(oracle.get_constant(true));

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("No inputs, two constant outputs (one negated)")
    {
        auto oracle = mockturtle::xag_network();
        oracle.create_po(oracle.get_constant(false));
        oracle.create_po(oracle.get_constant(true));

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("Two inputs, two constant outputs (one negated)")
    {
        auto oracle = mockturtle::xag_network();
        oracle.create_pi();
        oracle.create_pi();
        oracle.create_po(oracle.get_constant(true));
        oracle.create_po(oracle.get_constant(false));

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}

TEST_CASE("Synthesize buffer gate", "[xag][synth]")
{
    using namespace tweedledum;
    SECTION("One input, one output")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        oracle.create_po(a);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("One input, one nagated output")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        oracle.create_po(a ^ 1);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("One input, two outputs (one nagated)")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        oracle.create_po(a ^ 1);
        oracle.create_po(a);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}

TEST_CASE("Synthesize one AND gate", "[xag][synth]")
{
    using namespace tweedledum;
    SECTION("Simple AND")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a, b);
        oracle.create_po(ab);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("Negated output")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a, b);
        oracle.create_po(ab ^ 1);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("Nagated input a")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a ^ 1, b);
        oracle.create_po(ab);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("Nagated input b")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a, b ^ 1);
        oracle.create_po(ab);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("Nagated inputs")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a ^ 1, b ^ 1);
        oracle.create_po(ab);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("Nagated inputs and output")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a ^ 1, b ^ 1);
        oracle.create_po(ab ^ 1);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("'Copied' output")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a, b);
        oracle.create_po(ab);
        oracle.create_po(ab);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("'Copied' and nagated output 0")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a, b);
        oracle.create_po(ab ^ 1);
        oracle.create_po(ab);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("'Copied' and nagated output 1")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a, b);
        oracle.create_po(ab);
        oracle.create_po(ab ^ 1);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}

TEST_CASE("Synthesize one XOR gate", "[xag][synth]")
{
    using namespace tweedledum;
    SECTION("Simple XOR")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto a_xor_b = oracle.create_xor(a, b);
        oracle.create_po(a_xor_b);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("Negated output")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto a_xor_b = oracle.create_xor(a, b);
        oracle.create_po(a_xor_b ^ 1);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("'Copied' output")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto a_xor_b = oracle.create_xor(a, b);
        oracle.create_po(a_xor_b);
        oracle.create_po(a_xor_b);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("'Copied' and negated output 0")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto a_xor_b = oracle.create_xor(a, b);
        oracle.create_po(a_xor_b ^ 1);
        oracle.create_po(a_xor_b);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
    SECTION("'Copied' and negated output 1")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto a_xor_b = oracle.create_xor(a, b);
        oracle.create_po(a_xor_b);
        oracle.create_po(a_xor_b ^ 1);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}

TEST_CASE("Synthesize AND-XOR gate", "[xag][synth]")
{
    using namespace tweedledum;
    SECTION("Simple XOR")
    {
        auto oracle = mockturtle::xag_network();
        auto a = oracle.create_pi();
        auto b = oracle.create_pi();
        auto ab = oracle.create_and(a, b);
        auto ab_xor_b = oracle.create_xor(ab, b);
        oracle.create_po(ab_xor_b);

        Circuit circuit = xag_synth(oracle);
        auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
        auto const miter =
          *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}

// I'm cannot remember where I run into this case
TEST_CASE("An edge case", "[xag][synth]")
{
    using namespace tweedledum;
    auto oracle = mockturtle::xag_network();
    auto x0 = oracle.create_pi();
    auto x3 = oracle.create_pi();
    auto x4 = oracle.create_pi();
    auto x5 = oracle.create_pi();
    auto x6 = oracle.create_pi();
    auto n10 = oracle.create_xor(x6, x0);
    auto n9 = oracle.create_xor(x5, x3);
    auto n16 = oracle.create_xor(n10, n9);
    auto n20 = oracle.create_xor(n16, x4);
    auto n30 = oracle.create_and(x0, x3);
    auto n31 = oracle.create_and(n16 ^ 1, n30);
    auto n32 = oracle.create_and(n31, n20 ^ 1);
    oracle.create_po(n32);
    oracle.create_po(n32 ^ 1);
    oracle.create_po(n30);
    oracle.create_po(n32);
    oracle.create_po(oracle.get_constant(false));
    oracle.create_po(oracle.get_constant(true));
    oracle.create_po(x3 ^ 1);
    oracle.create_po(oracle.create_nary_xor({n30, n31, n32}));
    oracle.create_po(n30);

    Circuit circuit = xag_synth(oracle);
    auto xag = to_xag_network(circuit, oracle.num_pis(), oracle.num_pos());
    auto const miter = *mockturtle::miter<mockturtle::xag_network>(oracle, xag);
    auto const result = mockturtle::equivalence_checking(miter);
    CHECK(result);
    CHECK(*result);
}

TEST_CASE("Out-of-place adder", "[xag][synth]")
{
    using namespace mockturtle;
    using namespace tweedledum;
    for (uint32_t n = 2; n <= 8; ++n) {
        xag_network xag;
        std::vector<xag_network::signal> a(n);
        std::vector<xag_network::signal> b(n);
        std::generate(a.begin(), a.end(), [&xag]() { return xag.create_pi(); });
        std::generate(b.begin(), b.end(), [&xag]() { return xag.create_pi(); });
        auto carry = xag.create_pi();
        carry_ripple_adder_inplace(xag, a, b, carry);
        std::for_each(a.begin(), a.end(), [&](auto f) { xag.create_po(f); });
        xag.create_po(carry);

        Circuit circuit = xag_synth(xag);
        auto output = to_xag_network(circuit, xag.num_pis(), xag.num_pos());
        auto const miter = *mockturtle::miter<xag_network>(xag, output);
        auto const result = equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}

TEST_CASE("Out-of-place modular adder", "[xag][synth]")
{
    using namespace mockturtle;
    using namespace tweedledum;
    for (uint32_t n = 2; n <= 8; ++n) {
        xag_network xag;
        std::vector<xag_network::signal> a(n);
        std::vector<xag_network::signal> b(n);
        std::generate(a.begin(), a.end(), [&xag]() { return xag.create_pi(); });
        std::generate(b.begin(), b.end(), [&xag]() { return xag.create_pi(); });
        modular_adder_inplace(xag, a, b);
        std::for_each(a.begin(), a.end(), [&](auto f) { xag.create_po(f); });
        // FIXME: For some reason there are dangling nodes here!?
        xag = cleanup_dangling(xag);
        Circuit circuit = xag_synth(xag);
        auto output = to_xag_network(circuit, xag.num_pis(), xag.num_pos());
        auto const miter = *mockturtle::miter<xag_network>(xag, output);
        auto const result = equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}

TEST_CASE("Out-of-place multiplier", "[xag][synth]")
{
    using namespace mockturtle;
    using namespace tweedledum;
    for (uint32_t n = 2; n <= 8; ++n) {
        xag_network xag;
        std::vector<xag_network::signal> a(n);
        std::vector<xag_network::signal> b(n);
        std::generate(a.begin(), a.end(), [&xag]() { return xag.create_pi(); });
        std::generate(b.begin(), b.end(), [&xag]() { return xag.create_pi(); });
        for (auto const& o : carry_ripple_multiplier(xag, a, b)) {
            xag.create_po(o);
        }
        xag = xag_constant_fanin_optimization(xag);

        Circuit circuit = xag_synth(xag);
        auto output = to_xag_network(circuit, xag.num_pis(), xag.num_pos());
        auto const miter = *mockturtle::miter<xag_network>(xag, output);
        auto const result = mockturtle::equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}

TEST_CASE("Out-of-place Montgomery multiplier", "[xag][synth]")
{
    using namespace mockturtle;
    using namespace tweedledum;
    uint32_t const n = 6;
    xag_network xag;
    std::vector<xag_network::signal> a(n);
    std::vector<xag_network::signal> b(n);
    std::generate(a.begin(), a.end(), [&xag]() { return xag.create_pi(); });
    std::generate(b.begin(), b.end(), [&xag]() { return xag.create_pi(); });
    auto const outputs = montgomery_multiplication(xag, a, b, 17);
    for (auto const& o : outputs) {
        xag.create_po(o);
    }
    xag = xag_constant_fanin_optimization(xag);

    Circuit circuit = xag_synth(xag);
    auto output = to_xag_network(circuit, xag.num_pis(), xag.num_pos());
    auto const miter = *mockturtle::miter<xag_network>(xag, output);
    auto const result = equivalence_checking(miter);
    CHECK(result);
    CHECK(*result);
}

TEST_CASE("Out-of-place n-to-2^n binary decoder", "[xag][synth]")
{
    using namespace mockturtle;
    using namespace tweedledum;
    for (uint32_t n = 2; n <= 8; ++n) {
        xag_network xag;
        std::vector<xag_network::signal> xs(n);
        std::generate(xs.begin(), xs.end(), [&]() { return xag.create_pi(); });
        const auto ds = binary_decoder(xag, xs);
        std::for_each(
          ds.begin(), ds.end(), [&](auto const& d) { xag.create_po(d); });
        xag = xag_constant_fanin_optimization(xag);

        Circuit circuit = xag_synth(xag);
        auto output = to_xag_network(circuit, xag.num_pis(), xag.num_pos());
        auto const miter = *mockturtle::miter<xag_network>(xag, output);
        auto const result = equivalence_checking(miter);
        CHECK(result);
        CHECK(*result);
    }
}
