/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/mapping/nct.hpp>
#include <tweedledum/algorithms/synthesis/decomposition_based.hpp>
#include <tweedledum/algorithms/synthesis/single_target_gates.hpp>
#include <tweedledum/gates/gate_kinds.hpp>
#include <tweedledum/gates/mcst_gate.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/gg_network.hpp>

TEST_CASE("Check DBS with PRIME(3) and PPRM", "[decomposition_based]")
{
	using namespace tweedledum;
	std::vector<uint16_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
	const auto circ = decomposition_based_synthesis<netlist<mcmt_gate>>(permutation, stg_from_pprm());

	for (auto i = 0; i < 8; ++i) {
		CHECK(i == permutation[i]);
	}

	CHECK(circ.num_gates() == 6u);
	CHECK(circ.num_qubits() == 3u);
}

TEST_CASE("Check DBS with PRIME(3) and spectrum", "[decomposition_based]")
{
	using namespace tweedledum;
	std::vector<uint16_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
	const auto circ = decomposition_based_synthesis<gg_network<mcst_gate>>(permutation, stg_from_spectrum());

	for (auto i = 0; i < 8; ++i) {
		CHECK(i == permutation[i]);
	}

	CHECK(circ.num_gates() == 48u);
	CHECK(circ.num_qubits() == 3u);
}
