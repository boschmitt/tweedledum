/*--------------------------------------------------------------------------------------------------
| This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
| Author(s): Bruno Schmitt
*-------------------------------------------------------------------------------------------------*/
#include <catch.hpp>
#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <sstream>
#include <tweedledum/algorithms/synthesis/stg.hpp>
#include <tweedledum/gates/mcmt_gate.hpp>
#include <tweedledum/networks/netlist.hpp>
#include <tweedledum/networks/qubit.hpp>

using namespace tweedledum;

namespace tweedledum::detail {

template<class Network>
inline auto circuit_and_map(uint32_t qubits)
{
	Network network;
	for (auto i = 0u; i < qubits; ++i) {
		network.add_qubit();
	}
	std::vector<qubit_id> map(qubits);
	std::iota(map.begin(), map.end(), 0u);
	return std::make_pair(network, map);
}

} // namespace tweedledum::detail

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_pkrm",
          "[stg]")
{
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	auto [network, map] = detail::circuit_and_map<netlist<mcmt_gate>>(6u);
	stg_from_pkrm()(network, map, tt);

	CHECK(network.num_gates() == 40u);
	CHECK(network.num_qubits() == 6u);
}

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_pprm",
          "[stg]")
{
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	auto [network, map] = detail::circuit_and_map<netlist<mcmt_gate>>(6u);
	stg_from_pprm()(network, map, tt);

	CHECK(network.num_gates() == 21u);
	CHECK(network.num_qubits() == 6u);
}

TEST_CASE("Synthesize a simple function into a quantum network using stg_from_spectrum",
          "[stg]")
{
	kitty::dynamic_truth_table tt(5);
	kitty::create_from_hex_string(tt, "DA657041");
	auto [network, map] = detail::circuit_and_map<netlist<mcmt_gate>>(6u);
	stg_from_spectrum()(network, map, tt);

	CHECK(network.num_gates() == 185u);
	CHECK(network.num_qubits() == 6u);
}
