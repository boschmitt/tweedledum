/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Mathias Soeken, Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <cstdint>
#include <tweedledum/algorithms/synthesis/tbs.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <vector>


TEST_CASE("Synthesize PRIME(3) with unidirectional transformation based synthesis",
          "[tbs]")
{
	using namespace tweedledum;
	std::vector<uint32_t> permutation = {0, 2, 3, 5, 7, 1, 4, 6};
	const auto network = tbs<netlist<mcmt_gate>>(permutation);

	CHECK(network.num_gates() == 4u);
	CHECK(network.num_qubits() == 3u);
}

TEST_CASE("Synthesize PRIME(3) with bidirectional transformation based synthesis",
          "[tbs]")
{
	using namespace tweedledum;
	std::vector<uint32_t> permutation = {0, 2, 3, 5, 7, 1, 4, 6};
	tbs_params params;
	params.behavior = tbs_params::behavior::bidirectional;
	const auto network = tbs<netlist<mcmt_gate>>(permutation);

	CHECK(network.num_gates() == 4u);
	CHECK(network.num_qubits() == 3u);
}

TEST_CASE("Synthesize PRIME(3) with multi-directional transformation based synthesis",
          "[tbs]")
{
	using namespace tweedledum;
	std::vector<uint32_t> permutation = {0, 2, 3, 5, 7, 1, 4, 6};
	tbs_params params;
	params.behavior = tbs_params::behavior::multidirectional;
	const auto network = tbs<netlist<mcmt_gate>>(permutation, params);

	CHECK(network.num_gates() == 4u);
	CHECK(network.num_qubits() == 3u);
}
