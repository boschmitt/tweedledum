/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <mockturtle/algorithms/equivalence_checking.hpp>
#include <mockturtle/algorithms/miter.hpp>
#include <mockturtle/algorithms/simulation.hpp>
#include <mockturtle/networks/xag.hpp>
#include <tweedledum/algorithms/generic/to_logic_network.hpp>
#include <tweedledum/algorithms/optimization/gate_cancellation.hpp>
#include <tweedledum/algorithms/synthesis/oracles/hrs.hpp>
#include <tweedledum/gates/io3_gate.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/io/write_unicode.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/io_id.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace mockturtle;
using namespace tweedledum;

TEMPLATE_PRODUCT_TEST_CASE("Simple XAG synthesis", "[oracle_synthesis][template]",
                           (gg_network, netlist), (io3_gate, mcmt_gate))
{
	auto oracle = xag_network();
	auto a = oracle.create_pi();
	auto b = oracle.create_pi();
	auto a_and_b = oracle.create_and(a, b);
	oracle.create_po(a_and_b);

	TestType quantum_ntk;
	hrs_info info;
	hrs(quantum_ntk, oracle, &info);
	// write_unicode(quantum_ntk);
}

TEMPLATE_PRODUCT_TEST_CASE("Simple XAG synthesis 2", "[oracle_synthesis][template]",
                           (gg_network), (io3_gate))
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
	hrs_info info;
	hrs(quantum_ntk, oracle, &info);
	quantum_ntk = gate_cancellation(quantum_ntk);
	write_unicode(quantum_ntk);

	auto out_network = to_logic_network<mockturtle::xag_network>(quantum_ntk, info.inputs, info.outputs);
	const auto miter = *mockturtle::miter<mockturtle::xag_network>(oracle, out_network);
	const auto result = mockturtle::equivalence_checking(miter);
	CHECK(result);
	CHECK(*result);
}

TEMPLATE_PRODUCT_TEST_CASE("Simple XAG synthesis 3", "[oracle_synthesis][template]",
                           (gg_network), (io3_gate))
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
	auto n33 = oracle.create_and(n31, n31);
	oracle.create_po(n32);
	oracle.create_po(n32 ^ 1);
	oracle.create_po(n32);
	oracle.create_po(oracle.get_constant(false));
	oracle.create_po(x3 ^ 1);
	oracle.create_po(n33);

	TestType quantum_ntk;
	hrs_info info;
	hrs(quantum_ntk, oracle, &info);
	// quantum_ntk = gate_cancellation(quantum_ntk);
	write_unicode(quantum_ntk);

	auto out_network = to_logic_network<mockturtle::xag_network>(quantum_ntk, info.inputs, info.outputs);
	const auto miter = *mockturtle::miter<mockturtle::xag_network>(oracle, out_network);
	const auto result = mockturtle::equivalence_checking(miter);
	CHECK(result);
	CHECK(*result);
}
