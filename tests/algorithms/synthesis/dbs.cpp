/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt, Mathias Soeken
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/synthesis/dbs.hpp>
#include <tweedledum/algorithms/synthesis/stg.hpp>
#include <tweedledum/gates/gate_set.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/gg_network.hpp>
#include <tweedledum/networks/netlist.hpp>

using namespace tweedledum;
TEMPLATE_PRODUCT_TEST_CASE("Decomposition based synthesis", "[dbs][template]",
                           (gg_network, netlist), (mcmt_gate))
{
	std::vector<uint32_t> permutation = {0, 2, 3, 5, 7, 1, 4, 6};
	SECTION("Synthesize PRIME(3) - PPRM")
	{
		const auto network = dbs<TestType>(permutation, stg_from_pprm());
		CHECK(network.num_gates() == 6u);
		CHECK(network.num_qubits() == 3u);
	}
	SECTION("Synthesize PRIME(3) - spectrum")
	{
		const auto network = dbs<TestType>(permutation, stg_from_spectrum());
		CHECK(network.num_gates() == 48u);
		CHECK(network.num_qubits() == 3u);
	}
}

// TEST_CASE("Check DBS with PRIME(3) and PPRM", "[dbs]")
// {
// 	using namespace tweedledum;
// 	std::vector<uint32_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
// 	const auto network = dbs<netlist<mcmt_gate>>(permutation, stg_from_pprm());

// 	CHECK(network.num_gates() == 6u);
// 	CHECK(network.num_qubits() == 3u);
// }

// TEST_CASE("Check DBS with PRIME(3) and spectrum", "[dbs]")
// {
// 	using namespace tweedledum;
// 	std::vector<uint32_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
// 	const auto network = dbs<netlist<mcmt_gate>>(permutation, stg_from_spectrum());

// 	CHECK(network.num_gates() == 48u);
// 	CHECK(network.num_qubits() == 3u);
// }