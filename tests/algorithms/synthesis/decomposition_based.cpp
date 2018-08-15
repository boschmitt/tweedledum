/*------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken
*-----------------------------------------------------------------------------*/
#include <catch.hpp>
#include <tweedledum/algorithms/mapping/nct.hpp>
#include <tweedledum/algorithms/synthesis/decomposition_based.hpp>
#include <tweedledum/algorithms/synthesis/single_target_gates.hpp>
#include <tweedledum/io/quil.hpp>
#include <tweedledum/io/write_projectq.hpp>
#include <tweedledum/networks/gates/mct_gate.hpp>
#include <tweedledum/networks/gates/qc_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/dag_path.hpp>

TEST_CASE("Check DBS with PRIME(3) and PPRM", "decomposition_based")
{
	using namespace tweedledum;
	netlist<mct_gate> network;
	std::vector<uint16_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
	decomposition_based_synthesis(network, permutation, stg_from_pprm());

	dag_path<qc_gate> qnetwork;
	nct_mapping(qnetwork, network);
	write_projectq(network, std::cout);
	write_quil(qnetwork, std::cout);
	std::cout << "\n";
}

TEST_CASE("Check DBS with PRIME(3) and spectrum", "decomposition_based")
{
	using namespace tweedledum;
	dag_path<qc_gate> network;
	std::vector<uint16_t> permutation{{0, 2, 3, 5, 7, 1, 4, 6}};
	decomposition_based_synthesis(network, permutation, stg_from_spectrum());

	write_quil(network, std::cout);
}
