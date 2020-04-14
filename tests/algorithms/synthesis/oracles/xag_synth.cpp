/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
// FIXME: There are conflicts betweend SAT solver, so this header need to appear first! Quite wierd!
#include <mockturtle/algorithms/equivalence_checking.hpp>

#include "tweedledum/algorithms/synthesis/oracles/xag_synth.hpp"

#include "tweedledum/algorithms/utility/to_logic_network.hpp"
#include "tweedledum/operations/w3_op.hpp"
#include "tweedledum/operations/wn32_op.hpp"
#include "tweedledum/io/write_utf8.hpp"
#include "tweedledum/networks/netlist.hpp"
#include "tweedledum/networks/op_dag.hpp"

#include <catch.hpp>
#include <mockturtle/algorithms/miter.hpp>
#include <mockturtle/networks/xag.hpp>

using namespace mockturtle;
using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Simple XAG synthesis", "[oracle_synthesis][template]",
                           (op_dag, netlist), (w3_op, wn32_op))
{
	auto oracle = xag_network();
	auto a = oracle.create_pi();
	auto b = oracle.create_pi();
	auto a_and_b = oracle.create_and(a, b);
	oracle.create_po(a_and_b);

	TestType quantum_ntk;
	xag_synth(quantum_ntk, oracle);
}

TEMPLATE_PRODUCT_TEST_CASE("Simple XAG synthesis 2", "[oracle_synthesis][template]",
                           (op_dag), (w3_op))
{
	auto oracle = xag_network();
	auto a = oracle.create_pi();
	auto b = oracle.create_pi();
	auto c = oracle.create_pi();
	auto d = oracle.create_pi();
	auto e = oracle.create_pi();
	auto n0 = oracle.create_xor(d, b);
	auto n1 = oracle.create_and(e ^ 1, n0);
	auto n2 = oracle.create_xor(n1, b);
	auto n3 = oracle.create_xor(c, a);
	auto n4 = oracle.create_and(e ^ 1, n3);
	auto n5 = oracle.create_xor(n4, a);
	auto n6 = oracle.create_xor(n2, n5);
	oracle.create_po(n6); 

	TestType quantum_ntk;
	xag_synth(quantum_ntk, oracle);

	auto out_network = to_logic_network<mockturtle::xag_network>(quantum_ntk);
	const auto miter = *mockturtle::miter<mockturtle::xag_network>(oracle, out_network);
	const auto result = mockturtle::equivalence_checking(miter);
	CHECK(result);
	CHECK(*result);
}

TEMPLATE_PRODUCT_TEST_CASE("Simple XAG synthesis 3", "[oracle_synthesis][template]", (op_dag),
                           (w3_op))
{
	/* Test includeness */
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
	oracle.create_po(n32);
	oracle.create_po(oracle.get_constant(false));
	oracle.create_po(x3 ^ 1);

	TestType quantum_ntk;
	xag_synth(quantum_ntk, oracle);
	auto out_network = to_logic_network<mockturtle::xag_network>(quantum_ntk);
	CHECK(out_network.num_pis() == 5u);
	CHECK(out_network.num_pos() == 5u);

	const auto miter = *mockturtle::miter<mockturtle::xag_network>(oracle, out_network);
	const auto result = mockturtle::equivalence_checking(miter);
	CHECK(result);
	CHECK(*result);
}
